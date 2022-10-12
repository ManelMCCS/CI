// #include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "modbusAP.h"
#include "modbustcp.h"

#define SERVER_PORT 5502
#define IP_ADDR_SERVER "127.0.0.1"
#define NUM_CONNECTIONS 10

#define HOLD_REG_BOTTOM 0
#define HOLD_REG_TOP 99

uint16_t holding_registers[HOLD_REG_TOP - HOLD_REG_BOTTOM + 1] = {0};

/******************
 * ERROR HANDLING *
 ******************/
#define ILLG_FUNCTION 0x01
#define DATA_ADDRESS 0x02
#define DATA_VALUE 0x03

#define BUFFER_VALUES_W 100

/*********************
 * FUNCTIONS HEADERS *
 *********************/
int server_connect(struct sockaddr_in *server_add, int port);
int server_disconnect(int fd);
int R_h_regs(uint16_t st, uint16_t n, uint16_t *val);
int W_regs(uint16_t st, uint16_t n, uint16_t *val);

int main(void)
{
    int sock_server,
        sock_data,
        transaction_ident,
        var_aux,
        count;
    struct sockaddr_in local, remote;
    uint8_t function_operation,
        error_apdu;
    uint16_t number_reg;
    uint16_t values[BUFFER_VALUES_W];
    uint32_t start_address;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    sock_server = server_connect(&local, PROTOCOL_PORT);
    if (0 > sock_server)
    {
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        transaction_ident = Get_request(
            sock_server,
            &function_operation,
            &start_address,
            &number_reg,
            values);

        switch (function_operation) // function operation from Get_request ?
        {
        case R_HOLD_REGS:
            var_aux = R_h_regs(start_address, number_reg, values);
            if (var_aux < 0)
            {
                function_operation = function_operation + 0x08;
                error_apdu = DATA_ADDRESS;
            }
            break;

        case W_MULT_REGS:
            var_aux = W_regs(start_address, number_reg, values);
            if ((-1 == var_aux) || (-3 == var_aux))
            {
                function_operation = function_operation + 0x08;
                error_apdu = DATA_ADDRESS;
            }
            else if ((-2 == var_aux) || (-4 == var_aux))
            {
                function_operation = function_operation + 0x08;
                error_apdu = DATA_ADDRESS;
            }
            break;

        default:
            function_operation = function_operation + 0x08;
            error_apdu = ILLG_FUNCTION;
        }
    }

    server_disconnect(sock_server);
}

/**
 * @brief Connect to the server
 *
 * @param server_add struct with the server address
 * @param port connection port number
 * @return int
 */
int server_connect(struct sockaddr_in *server_add, int port)
{
    if (NULL == server_add)
    {
        return -1;
    }
    else if ((PORT_MIN > port) || (PORT_MAX < port))
    {
        return -2;
    }

    int connection_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == connection_socket)
    {
        return -3;
    }

    // binds server address
    server_add->sin_family = AF_INET;
    server_add->sin_port = htons(port);
    if (0 == inet_aton(IP_ADDR_SERVER, &server_add->sin_addr))
    {
        close(connection_socket);
        return -4;
    }
    if (-1 == bind(connection_socket, (struct sockaddr *)server_add, sizeof(struct sockaddr_in)))
    {
        close(connection_socket);
        return -5;
    }

    // listen - prepares for requests
    if (-1 == listen(connection_socket, NUM_CONNECTIONS))
    {
        close(connection_socket);
        return -6;
    }

    // returns fd - ok, <0 - error
    return connection_socket;
}

int server_disconnect(int fd)
{
    // closes / destroys control socket
    int return_value = close(fd);

    // returns >0 - ok, <0 - error
    if (0 != return_value)
    {
        return -1;
    }
    else
        return 0;
}

/**
 * @brief Read holding registers
 *
 * @param st
 * @param n
 * @param val
 * @return int
 */
int R_h_regs(uint16_t st, uint16_t n, uint16_t *val)
{
    st--;
    if (HOLD_REG_BOTTOM > st || HOLD_REG_TOP < st)
    {
        return -1;
    }
    else if (HOLD_REG_TOP < st + n - 1)
    {
        printf("[SERVER][ERROR 2] R_h_regs\n");
        return -2;
    }

    // read n regs starting from st and write in val
    int count;
    for (count = 0; count < n; count++)
        val[count] = holding_registers[count + st];

    // returns: num regs read, values in val - ok, <0 - error
    return count;
}

int W_regs(uint16_t st, uint16_t n, uint16_t *val)
{
    st--;
    if (HOLD_REG_BOTTOM > st || HOLD_REG_TOP < st)
    {
        return -1;
    }
    else if (NULL == val)
    {
        return -2;
    }
    else if (HOLD_REG_TOP < st + n - 1)
    {
        return -3;
    }

    // write n regs starting from st using values from val
    int count;
    for (count = 0; (count < n) && (count + st < HOLD_REG_TOP); count++)
    {
        holding_registers[count + st] = val[count];
    }

    // returns: num regs written - ok, <0 - error
    if (count < n)
    {
        return -4;
    }
    else
        return count;
}