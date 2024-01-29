#include "os.h"

#include "control.h"
#include "sensors.h"
#include "actuators.h"
#include "architecture.h"



float aux_speed = 0; //iniciamos velocidad auxiliar                    
int read_sensor(sensor_t *sensor)
{
/*  
    Tarea encargada de leer un sensor y guardar en 'value' el resultado de
    dicho sensor.

    Adicionalmente, estimará la velocidad sabiendo que el tiempo que tardamos entre
    una lectura y otra es 300 ms.
    
    DELAY: 300ms
    RETURN: 0 cuando se ha obtenido un resultado, recordamos que 'read' es bloqueante
*/
    //GUARDAMOS VALOR ANTERIOR
    sensor->value_ant = sensor->value;

    //OBTENEMOS NUEVO VALOR
    recv(sensor->fd,&sensor->value,sizeof(float),0);

    if (sensor -> status < 4){
        sensor->status++;
    }
    else
    {
        switch (sensor->status)
        {
            case  4:
                sensor->status++;
                break;
            case 5 :
            //ESTIMAMOS VELOCIDAD
                sensor->speed = (1000*(sensor->value_ant - sensor->value))/SENSOR_CYCLE; 
                sensor->status++;
                break;
            case 6 : 
            //MIRAMOS SI HAY ACELERACIÓN Y DECLARAMOS MODO DE MOVIMIENTO
                aux_speed = sensor ->speed ;
                sensor ->speed = (1000*(sensor->value_ant - sensor->value))/SENSOR_CYCLE; 
                sensor -> acceleration = ((sensor->speed - aux_speed)/SENSOR_CYCLE);
                if (sensor->acceleration != 0) sensor -> type = ACCELERATION_MODE ;
                else if (sensor ->speed != 0) sensor -> type = VEL_MODE ;
                else sensor -> type = STATIC_MODE ;
                sensor->status++;
                break;
            case 7: 
            //VAMOS ACTUALIZANDO VELOCIDAD RELATIVA
                sensor->speed = (1000*(sensor->value_ant - sensor->value))/SENSOR_CYCLE;  
                break;
        }
    }
    return 0;
}

int write_actuator(actuator_t *actuator)
{
/*
    Función encargada de modificar el estado del actuador pasado como
    parámetro
    
    DELAY = 50ms
*/
    char c = 0;
    int length = strlen(actuator->message)+1;
    send(actuator->fd,(char*)&length,sizeof(int),0);
    send(actuator->fd,actuator->message,length,0);
    recv(actuator->fd,(char*)&c,sizeof(char),0);

    return 0;
}

int start_actuator(actuator_t *actuator)
{

    sprintf(actuator->message,"%s",start_message);
    return 0;
}

int stop_actuator(actuator_t *actuator)
{
    sprintf(actuator->message,"%s",stop_message);
    return 0;
}


int decisor_actuador(sensor_t *sensor, actuator_t* POSITIVE, actuator_t *NEGATIVE)
{
/*
    Función encargada de encender el motor correcto dada una distancia, sirve para
    los 3 ejes (N/S W/S UP/DOWN).

    El actuador POSITIVE es el que se debe activar si sensor->value > 0
    El actuador NEGATIVE es el que se debe activar si sensor->value < 0
    En caso de que sensor->value = 0, se mantendran apagados ambos actuadores
    
    Nota importante: si el signo de sensor->speed y sensor->value son iguales, quiere decir que nos acercamos
*/
    
    if (sensor ->type == STATIC_MODE || sensor ->type ==  VEL_MODE || sensor ->type == ACCELERATION_MODE)
    {
        DTS = 1.5*(0.5/(-0.2+sensor->acceleration))*(sensor->speed*sensor->speed)-1.5*((sensor->speed*SENSOR_CYCLE/1000)+ 0.5*(0.2-sensor->acceleration)*SENSOR_CYCLE/1000);
        
        if (sensor->value < 0 ) DTS = DTS - sensor ->value;
        else DTS = DTS + sensor ->value;
        
        if (DTS < 0) action = FRENAR;
        else action = ACELERAR;
        
        //Sabemos que la distancia de "acople" es de 5 metros, por lo que haremos frenar la sonda siempre que estemos dentro
        //de este alcance (usando 4 para dar cierto margen de error)
        if ((sensor->value < COUPLING_DIST && sensor->value > -COUPLING_DIST)) action = FRENAR;

        //Caso en que la nave se nos escapa, en cualquier situación nos va a interesar acelerar hacia ella
        if (((sensor->value < 0 && sensor->speed > 0) || (sensor->value > 0 && sensor->speed < 0)) && (sensor->value > COUPLING_DIST || sensor->value < -COUPLING_DIST) ){
            printf("Se nos escapa la nave, ACELERANDO!");
            if (sensor->value > 0)
            {
                start_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }

            else if (sensor->value < 0)
            {
                stop_actuator(POSITIVE);
                start_actuator(NEGATIVE);
            }

            else
            {
                stop_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }
        }
        
        else if ((sensor->speed > -1 && sensor->speed < 1) && (sensor -> value < -COUPLING_DIST && sensor ->value > COUPLING_DIST))
        {
            printf("ACELERANDO PARA EL OBJETIVO");
            if (sensor->value > 0)
            {
                start_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }

            else if (sensor->value < 0)
            {
                stop_actuator(POSITIVE);
                start_actuator(NEGATIVE);
            }

            else
            {
                stop_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }
        } 
        
        else if (action == FRENAR){
            printf("Frenando ");
            if (sensor->speed < 0)
            {
                start_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }

            else if (sensor->speed > 0)
            {
                stop_actuator(POSITIVE);
                start_actuator(NEGATIVE);
            }

            else
            {
                stop_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }

        }
        else if (action == ACELERAR)
        {
            printf("ACELERANDO ");
            if (sensor->value > 0)
            {
                start_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }

            else if (sensor->value < 0)
            {
                stop_actuator(POSITIVE);
                start_actuator(NEGATIVE);
            }

            else
            {
                stop_actuator(POSITIVE);
                stop_actuator(NEGATIVE);
            }
        }
    }
    printf("Speed: %.2f\tVal: %.2f\tDTS: %.2f\n",sensor->speed,sensor->value,DTS);
    
    write_actuator(POSITIVE);
    write_actuator(NEGATIVE);

    return 0;

}


int control_eje_X()
{
    system("clear");
    //Nos aseguramos que conocemos el tipo de movimiento que sigue Luke antes de
    //hacer ningun movimiento
    if(sensor_X.type != UNKNOWN && sensor_Y.type != UNKNOWN && sensor_Z.type != UNKNOWN){
        printf("\rWE\t");
        decisor_actuador(&sensor_X,&actuator_W, &actuator_E);
    }

    return 0;
}

int control_eje_Y()
{
    system("clear");
    //Nos aseguramos que conocemos el tipo de movimiento que sigue Luke antes de
    //hacer ningun movimiento
    if(sensor_X.type != UNKNOWN && sensor_Y.type != UNKNOWN && sensor_Z.type != UNKNOWN){
        printf("\rNS\t");
        decisor_actuador(&sensor_Y,&actuator_N, &actuator_S);
    }

    return 0;
}

int control_eje_Z()
{ 
    system("clear");
    //Nos aseguramos que conocemos el tipo de movimiento que sigue Luke antes de
    //hacer ningun movimiento
    if(sensor_X.type != UNKNOWN && sensor_Y.type != UNKNOWN && sensor_Z.type != UNKNOWN){
        printf("\rUD\t");
        decisor_actuador(&sensor_Z,&actuator_UP, &actuator_DOWN);
    }
        
    return 0;
}

void init_structs()
{
    /*
        Función cuyo único propósito es establecer las condiciones
        iniciales de las estructuras usadas para los sensores y actuadores
    */
    sensor_X.fd = delta_X;
    sensor_Y.fd = delta_Y;
    sensor_Z.fd = delta_Z;
    sensor_X.value = 0;
    sensor_Y.value = 0;
    sensor_Z.value = 0;
    sensor_X.value_ant = 0;
    sensor_Y.value_ant = 0;
    sensor_Z.value_ant = 0;
    sensor_X.speed = 0;
    sensor_Y.speed = 0;
    sensor_Z.speed = 0;
    sensor_X.type = UNKNOWN;
    sensor_Y.type = UNKNOWN;  
    sensor_Z.type = UNKNOWN;
    
    actuator_N.fd = pump_N;
    actuator_S.fd = pump_S;
    actuator_E.fd = pump_E;
    actuator_W.fd = pump_W;
    actuator_UP.fd = pump_UP;
    actuator_DOWN.fd = pump_DOWN;
}


int init_tasks()
{
    //inicializamos los campos de las estructuras usadas para el control
    init_structs();
    
    //Generamos tareas para la lectura PERIODICA de los sensores, sabemos que su delay
    //de lectura es de 300ms
    
    PTASK read_sensor_X = NULL;
    Task_create(&read_sensor_X,"Read Sensor X",(void*)read_sensor,&sensor_X,SENSOR_CYCLE,SENSOR_DELAY);
    Readyqueue_enqueue(tasks_queue_X, read_sensor_X);
    
    PTASK read_sensor_Y = NULL;
    Task_create(&read_sensor_Y,"Read Sensor Y",(void*)read_sensor,&sensor_Y,SENSOR_CYCLE,SENSOR_DELAY);
    Readyqueue_enqueue(tasks_queue_Y, read_sensor_Y);
    
    PTASK read_sensor_Z = NULL;
    Task_create(&read_sensor_Z,"Read Sensor Z",(void*)read_sensor,&sensor_Z,SENSOR_CYCLE,SENSOR_DELAY);
    Readyqueue_enqueue(tasks_queue_Z, read_sensor_Z);

    //tarea encargada de actualizar todos los actuadores, por cada actuador esperamos un delay de 50ms,
    //dado que se accede a 2 actuadores por ciclo --> 100 ms, como no tenemos nuevos datos hasta pasado
    //un ciclo de read_sensor pondremos el mismo ciclo

    PTASK actuator_X = NULL;
    Task_create(&actuator_X,"Engines X Axis",(void*)control_eje_X,NULL,SENSOR_CYCLE,2*ACTUATOR_DELAY);
    Readyqueue_enqueue(tasks_queue_X, actuator_X);

    PTASK actuator_Y = NULL;
    Task_create(&actuator_Y,"Engines Y Axis",(void*)control_eje_Y,NULL,SENSOR_CYCLE,2*ACTUATOR_DELAY);
    Readyqueue_enqueue(tasks_queue_Y, actuator_Y);

    PTASK actuator_Z = NULL;
    Task_create(&actuator_Z,"Engines Z Axis",(void*)control_eje_Z,NULL,ACTUATOR_CYCLE,2*ACTUATOR_DELAY);
    Readyqueue_enqueue(tasks_queue_Z, actuator_Z);
    
    return 0;
}
