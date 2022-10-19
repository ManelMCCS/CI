#include "modbusTCP.h"

// The MBAP header is 7 bytes long: Transaction Identifier. It is used for transaction pairing.

#define MBAP_SIZE 7 // MBAPDU header size
#define UNIT_ID 52  // Slave ID - see server configuration

/**
 * @brief Send client's request from server
 *
 * @param server_add struct contaning the server address
 * @param port connection port
 * @param APDU stream to be sent
 * @param APDUlen APDU lenght
 * @param APDU_R APDU server response
 * @return int return MODBUS_ERROR (-1) in error case; return MODBUS_SUCCESS in success case
 */
int send_modbus_request(char *server_add, unsigned int port, uint8_t *APDU, uint16_t APDUlen, uint8_t *APDU_R)
{
    if (NULL == server_add)
    {
        return MODBUS_ERROR;
    }
    else if (NULL == APDU)
    {
        return MODBUS_ERROR;
    }

    int modbus_socket = socket(PF_INET, SOCK_STREAM, 0); // create socket var (int)
    if (modbus_socket < 0)
    {
        return MODBUS_ERROR;
    }

    struct sockaddr_in server; // socket address structure, stores the ports and address form AF_INET

    // Set port and IP of the server
    server.sin_family = AF_INET;             // AF_INET is the address family and is used to designate the type of address the socket can communicate with.
    server.sin_port = htons(port);           // Converts host's variable into a TCP/IP ip address.
    inet_aton(server_add, &server.sin_addr); // Converts IPv4 address into binary numbers-and-dots

    // try connecting with server:
    // Retorna um erro caso não se consiga ligar ao servidor e acabar o tempo de timeout.
    if (connect(modbus_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        return MODBUS_ERROR;
    }

    // build MBAPDU header (MBAP)
    unsigned char MBAP[MBAP_SIZE]; // O máximo do array MBAP pode ter é 7 bytes

    uint16_t one_bit_mask = 1;

    MBAP[0] = (uint8_t)(one_bit_mask >> 8);   // assembles the pdu's mbap
    MBAP[1] = (uint8_t)(one_bit_mask & 0xFF); // Low Byte (1111 1111) bit mask
    MBAP[2] = 0x00;                           // protocol identifier - 0
    MBAP[3] = 0x00;
    MBAP[4] = (uint8_t)((APDUlen + 1) >> 8);   // Hight Byte bit mask
    MBAP[5] = (uint8_t)((APDUlen + 1) & 0xFF); // Low Byte (1111 1111) bit mask
    MBAP[6] = UNIT_ID;

    // send request MBAPDU header (MBAP)
    if (write(modbus_socket, MBAP, MBAP_SIZE) < 0)
    {
        return MODBUS_ERROR;
    }

    if (write(modbus_socket, APDU, APDUlen) < 0)
    {
        return MODBUS_ERROR;
    }

    // receive response MBAPDU header (MBAP) - fixed size, reuzing "MBAP"
    if (read(modbus_socket, MBAP, MBAP_SIZE) < 0)
    {
        return MODBUS_ERROR;
    }

    // receive response MBAPDU payload (APDU_R) - get size from "Length" field
    int APDU_lenght = (MBAP[4] << 8) + MBAP[5]; // recover the "length" field from [4][5]

    // read TCP message from slave?
    if (read(modbus_socket, APDU_R, APDU_lenght - 1) < 0)
    {
        return -1;
    }
    // closes TCP client socket with server
    close(modbus_socket);

    return MODBUS_SUCCESS;
}