#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "modbusTCP.h"

int write_multiple_regs(char *, unsigned int, uint16_t, uint16_t, uint16_t *);
int read_h_regs(char *, unsigned int, uint16_t, uint16_t, uint16_t *);