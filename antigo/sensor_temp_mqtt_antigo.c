#include "contiki.h"

/********************libSENSORES*********************/
#include "dev/sht11/sht11-sensor.h"

#ifdef X_NUCLEO_IKS01A1
#include "dev/temperature-sensor.h"
#endif /*X_NUCLEO_IKS01A1*/

#include "mqtt-demo/mqtt.h"

/********************libLEDS*************************/
#include "dev/leds.h"
/****************************************************/

#include <stdio.h>

#define BROKER_IP "192.168.17.130"
#define BROKER_PORT 1883
#define CLIENT_ID "sensor-node"
#define MAX_TCP_SEGMENT_SIZE 32
static struct mqtt_connection mqtt_conn;

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "HELLO SENSORES");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/

//sht11-sensor
static int get_temp(void)
{
  return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
}

/*--------------------------LED---------------------------------------------*/
static void led_green(void)
{
	leds_on(LEDS_GREEN);
	leds_off(LEDS_BLUE);
	leds_off(LEDS_RED);
	
	printf("TEMPERATURA Ok (<=4°C)");
}

static void led_red(void)
{
	leds_off(LEDS_GREEN);
	leds_off(LEDS_BLUE);
	leds_on(LEDS_RED);
	
	printf("TEMPERATURA ALTA (>4°C)");
}

/*--------------------------CASE--------------------------------------------*/
static void sensor_avaliator(int temp){
    //CASES:

    if (temp <= 4){
    	led_green();
    } else {
	led_red();
    }
    printf("\n\n");  
}

PROCESS_THREAD(hello_world_process, ev, data)
{

    static struct etimer timer;
    int temp;

    PROCESS_BEGIN();

    SENSORS_ACTIVATE(sht11_sensor);
    etimer_set(&timer, 5 * CLOCK_SECOND);

    mqtt_register(&mqtt_conn, &hello_world_process, CLIENT_ID, NULL, MAX_TCP_SEGMENT_SIZE);
    mqtt_connect(&mqtt_conn, BROKER_IP, BROKER_PORT, 30);

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        
        temp = get_temp();
        sensor_avaliator(temp);
        
        etimer_reset(&timer);
    }

    SENSORS_DEACTIVATE(sht11_sensor);
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

