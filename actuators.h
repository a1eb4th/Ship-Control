#ifndef __ACTUATORS_H__
#define __ACTUATORS_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT_PUMP_N 20000
#define PORT_PUMP_S 20001
#define PORT_PUMP_E 20002
#define PORT_PUMP_W 20003
#define PORT_PUMP_UP 20004
#define PORT_PUMP_DOWN 20005

int init_actuators();

#endif
