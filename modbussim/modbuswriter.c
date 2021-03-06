/*****************************************************************************
 * 
 * c 2016 Whit Schonbein
 *
 *****************************************************************************
 *
 * Author       : Whit Schonbein (schonbein [at] cs.unm.edu)
 * Institution  : University of New Mexico, Albuquerque
 * Year         : 2016
 * Course       : cs544
 *
 ***************************************************************************** 
 *
 * Purpose:
 * 
 * Writes to a register via modbus protocol.
 *
 *****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <modbus.h>

int modbus_end(modbus_t *ctx) {
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}

int main(int argc, char **argv) {

    if (argc != 6) {
        fprintf(stdout, "\nUsage: %s <ip> <port> <register address (hex)> <16 bit value (hex)> <freq>\n\n", argv[0]);
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    uint16_t address = (uint16_t)strtoul(argv[3], NULL, 16);
    uint16_t value = (uint16_t)strtoul(argv[4], NULL, 16);
    int freq = atoi(argv[5]);
    uint16_t *register_values = malloc(sizeof(*register_values));

    fprintf(stdout, " ip = %s\n port = %d\n value = %X\n address = %X\n freq = %d\n", ip, port, value, address, freq);
    printf("If freq != 0, ctrl-c to quit\n");

    modbus_t *ctx = modbus_new_tcp(ip, port);
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection to %s port %d failed: %s\n", ip, port, modbus_strerror(errno));
        modbus_end(ctx);
        return -1;
    }

    while (1) {

        int rc = modbus_read_registers(ctx, 0, 1, register_values);
        if (-1 == rc) {
            fprintf(stderr, "Reading registers failed. %s\n", modbus_strerror(errno));
            modbus_end(ctx);
            return -1;
        }

        fprintf(stdout, "Before writing, the value of register at address %04x = %d\n", address, register_values[0]);

        int wc = modbus_write_register(ctx, (int)address, (int)value );
        if (-1 == wc) {
            fprintf(stderr, "Write of value %04x to address %04x failed: %s\n", value, address, modbus_strerror(errno));
            modbus_end(ctx);
            return -1;
        }
    
        rc = modbus_read_registers(ctx, (int)address, 1, register_values);
        if (-1 == rc) {
            fprintf(stderr, "Oh yeah this didn't work. %s\n", modbus_strerror(errno));
            modbus_end(ctx);
            return -1;
        }

        fprintf(stdout, "After writing, the value of register at address %04x = %d\n", address, register_values[0]);

        if (freq == 0) break;
        
        sleep(freq);
    }

    modbus_end(ctx);
    
    free(register_values);

    return 0;
}
