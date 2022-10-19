#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "modbusTCP.h"
#include "modbusAP.h"

// gcc ClienteM.c -c ClienteM.o
// gcc ClienteM.c ModbusAP.o ModbusTCP.o -o ClienteM.exe -> compilar todos os ficheiros e executar
// ou gcc ClienteM.o ModbusAP.o ModbusTCP.o -o ClienteM.exe -> Podemos fazer isto depois de compilar o ModbusAP e ModbusTCP

/*Vamos ao Modbus Poll e vamos ao settings -> Slay Definition -> Holding Registers (verificar se esta assim). Connect -> Register Now->Ok-> Ligação por ModbusTCP->Retirar a seta
do Unit_ID->Ok*/
// Depois temos apenas de executar o cliente -> ./clienteM.exe

#define SERVER_PORT 502 // porta standard 502
#define IN_BUF_LEN 256  // tamanho maximo do buffer 253 bytes -> 256
#define SERVER_ADDR "127.0.0.1"
#define SERVER_ADDR2 "10.227.113.1"

int main()
{

    uint16_t B, C, K;
    uint16_t A[3];

    // 1
    K = 0x44;

    if (write_multiple_regs(SERVER_ADDR, SERVER_PORT, 81, 1, &K) < 0)
    {
        return -1;
    }

    // 2
    if (read_h_regs(SERVER_ADDR, SERVER_PORT, 83, 3, A) < 0)
    {
        return -1;
    }

    printf(" %d %d %d\n", A[0], A[1], A[2]);
    // 3
    if (read_h_regs(SERVER_ADDR, SERVER_PORT, 82, 1, &B) < 0)
    {
        return -1;
    }

    printf(" %d\n", B);
    // 7

    if (B == 0)
    {
        C = 888;
    }
    else
    {
        C = A[1] - A[2];
    }

    // 4
    if (write_multiple_regs(SERVER_ADDR, SERVER_PORT, 86, 1, &C) < 0)
    {
        printf("[Client] Error: write_multiple_regs (2nd)\n");
        return -1;
    }

    // 5
    if (write_multiple_regs(SERVER_ADDR2, SERVER_PORT, 87, 1, &C) < 0)
    {
        printf("[Client] Error: write_multiple_regs (3st)\n");
        return -1;
    }

    return 0; // return zero in case of success
}
