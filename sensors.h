#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>


#define PORT_DELTA_X 20015
#define PORT_DELTA_Y 20016
#define PORT_DELTA_Z 20017

int init_sensors();

#endif
