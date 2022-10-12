#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MODBUS_ERROR -1
#define MODBUS_SUCCESS 0

int send_modbus_request(char *server_add, unsigned int port, uint8_t *APDU, uint16_t APDUlen, uint8_t *APDU_R);