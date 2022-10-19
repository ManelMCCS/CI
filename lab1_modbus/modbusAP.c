#include "modbusAP.h"

#define MAX_APDU 256
#define FC_WMR 16       // function code for WriteMultipleRegisters
#define FC_RHR 3        // function code for ReadHoldingRegisters
#define MAX_WR_REGS 123 // max number of registers that can be written
#define MAX_RD_REGS 125 // max number of registers that can be read

/**
 * @brief Reads the holding registers in the server.
 *
 * @param server_add struct that contains the server's address
 * @param port port used in the connection
 * @param st_r start address of the holding register (will send the offset relative value to 1)
 * @param n_r number of holding registers
 * @param val array containing the holding registers to be returned
 * @return int return MODBUS_ERROR (-1) in error case; return MODBUS_SUCCESS in success case
 */
int read_h_regs(char *server_add, unsigned int port, uint16_t st_r, uint16_t n_r, uint16_t *val)
{
    uint8_t APDU[MAX_APDU], APDU_R[MAX_APDU]; // criar o array de respostas APDU e APDU

    // verify if paramters are within proper ranges
    if (n_r > MAX_RD_REGS || n_r < 1)
    {
        return MODBUS_ERROR;
    }

    // build header
    APDU[0] = (uint8_t)FC_RHR;        // function code
    APDU[1] = (uint8_t)(st_r >> 8);   // high byte of start address st_r
    APDU[2] = (uint8_t)(st_r & 0xFF); // low byte
    APDU[3] = (uint8_t)(n_r >> 8);    // high byte of number of registers n_r
    APDU[4] = (uint8_t)(n_r & 0xFF);  // low byte
    APDU[5] = (uint8_t)(2 * n_r);     // High Byte->dobro do número de registos

    // send APDU to server, reuse "i" for return value and "APDU" fro response
    //  note APDUlen = 5 in this case (APDU is just the 5B header)

    int APDUlen = 5; // APDUlen = 1 + 2 + 2 (reply); check the slides for send modbus

    int modbus_request_status = send_modbus_request(server_add, port, APDU, APDUlen, APDU);

    // check non Modbus error
    // se isto aconteceu existiu um erro a enviar o pedido
    if (modbus_request_status < 0)
    {
        return MODBUS_ERROR;
    }

    // check Modbus error - Exception
    // Verificar a resposta do APDU_R o primeiro bit APDU_R[0].
    // se o i>=0, mas o primeiro bit do pacote de array de resposta APDU_R for 1 (1000 0000)-> 0x80 significa que existe um erro na resposta e a posição APDU_R[1] irá retorna o APDU_R[1]
    if (APDU[0] & 0x80)
    {
        return APDU[1];
    }

    // else place read values en val[] and return 0 - successfull
    // number od registers values = APDU[1]/2 --> B/2 (half of bytes)
    // values starting after byte 2 --> APDU[2] onward
    // move the payload (values from the registers) to the val array

    // nesta parte depois de verificar que não existiu erro nem a enviar nem no APDU_R vamos começar a preencher o array val
    for (size_t index = 0; index < APDU[1] / 2; index++)
    {
        // O parâmetro val de array vai ser a soma do High bytes do APDU de resposta com o Low bytes do APDU resposta. val é um array que receber os pacotes vindos do APDU_R.
        val[index] = (uint16_t)(APDU[2 + 2 * index] << 8) + (uint16_t)(APDU[3 + 2 * index]);
    }

    return 0;
}

/**
 * @brief Write holding registers to the server.
 *
 * @param server_add struct that contains the server's address
 * @param port port used in the connection
 * @param st_r start address of the holding register (will send the offset relative value to 1)
 * @param n_r number of holding registers
 * @param val array containing the holding registers to be returned
 * @return int return MODBUS_ERROR (-1) in error case; return MODBUS_SUCCESS in success case
 */
int write_multiple_regs(char *server_add, unsigned int port, uint16_t st_r, uint16_t n_r, uint16_t *val)
{
    // variables declaration
    uint8_t APDU[MAX_APDU], APDU_R[MAX_APDU];

    // verify if paramters are within proper ranges
    if (n_r > MAX_WR_REGS || n_r < 1)
    {
        return MODBUS_ERROR;
    }

    // build header
    APDU[0] = (uint8_t)FC_WMR;        // function code
    APDU[1] = (uint8_t)(st_r >> 8);   // high byte of start address st_r
    APDU[2] = (uint8_t)(st_r & 0xFF); // low byte
    APDU[3] = (uint8_t)(n_r >> 8);    // high byte of number of regs n_r
    APDU[4] = (uint8_t)(n_r & 0xFF);  // low byte
    APDU[5] = (uint8_t)2 * n_r;       // number of bytes in val (2B/reg)

    // build the payload with the values to be written in the registers
    for (size_t index = 0; index < n_r; index++)
    {
        APDU[6 + 2 * index] = (uint8_t)(val[index] >> 8);   // High Byte
        APDU[7 + 2 * index] = (uint8_t)(val[index] & 0xFF); // Low Byte
    }

    uint16_t APDUlen = 1 + 2 + 2 + 1 + 2 * n_r; // check the slides for write modbus

    int send_modbus_request_status = send_modbus_request(server_add, port, APDU, APDUlen, APDU);

    if (send_modbus_request_status < 0)
    {
        return MODBUS_ERROR;
    }

    // if i>=0 but the first bit of the APDU_R response array packet is 1 (1000 0000)-> 0x80 means there is an error in the response and the position APDU_R[1]
    if (APDU[0] & 0x80)
    {
        return APDU[1];
    }

    return 0;
}