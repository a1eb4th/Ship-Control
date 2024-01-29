#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <string.h>
#include <math.h>

#define SENSOR_DELAY 300
#define SENSOR_CYCLE 300
#define ACTUATOR_DELAY 50
#define ACTUATOR_CYCLE 300
#define MSG_SIZE 50
#define ACCELERATION_MODE 3
#define VEL_MODE 2
#define STATIC_MODE 1
#define UNKNOWN 0 
#define FRENAR 2
#define ACELERAR 1
#define COUPLING_DIST 4

extern int delta_X,delta_Y,delta_Z;                             //file descriptors de los sensores
extern int pump_N,pump_S,pump_E,pump_W,pump_UP,pump_DOWN;       //file descriptors de los actuadores

extern PREADYQUEUE tasks_queue_X, tasks_queue_Y, tasks_queue_Z; 

char * start_message = "start";
char * stop_message = "stop";



/*
        Mediante cinematica, y conociendo que la aceleración que nos proporcionan los actuadores es de a = 0.2 m/s, obtenemos
        el DTS (Distance To Stop), llamada así por el primer milestone. 
        El DTS nos permite conocer la distancia que necesitamos para llegar a la velocidad deseada.
        
        Adicionalmente, añadimos términos de error ya que sabemos que el DTS se obtiene con un retraso de SENSOR_CYCLE
        (300ms)
*/
float DTS = 0;

int action = UNKNOWN;
    




/* **************************   sensor_t   ***********************************

    Estructura encargada de almacenar datos referentes al sensor y "telemetria"
    obtenida directa e indicrectamente a partir de las medidas de los sensores,
    por ejemplo, velocidad y aceleración.
    
    Parametros:
    - fd:           File descriptor del socket correspondiente al sensor
    - value:        Valor obtenido por el sensor
    - value_ant:    Valor anterior obtenido por el sensor
    - speed:        Velocidad relativa estimada a partir de value y value_ant
    - acceleration: Aceleración relactiva estimada a partir de los anteriores parametros
    - Type:         Tipo de movimiento que sigue Luke en el eje correspondiente (velocidad nula, cte o acelerado)
    - Status:       Usado para saber en qué momento del calculo de velocidades y aceleracion nos encontramos,
                    inicia en 0 y termina en 7            
   ***************************************************************************** */
typedef struct{
    int fd;
    float value;
    float value_ant;
    float speed;
    float acceleration ;
    int type ;
    int status ;
}sensor_t;

typedef struct{
    int fd;
    char message[MSG_SIZE];
}actuator_t;


sensor_t sensor_X,sensor_Y,sensor_Z;                            
actuator_t actuator_N, actuator_S, actuator_E, actuator_W, actuator_UP, actuator_DOWN;

//Cabeceras de funciones empleadas en control.c
int read_sensor(sensor_t *sensor);
int write_actuator(actuator_t *actuator);
int decisor_actuador(sensor_t *sensor, actuator_t* POSITIVE, actuator_t *NEGATIVE);
int control_eje_X();
int control_eje_Y();
int control_eje_Z();
void init_structs();
int init_tasks();

#endif
