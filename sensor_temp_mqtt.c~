#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"

#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "net/netstack.h"

#include <stdio.h>

#include "dev/light-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "dev/temperature-sensor.h"

static struct collect_conn tc;

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process2, "Hello world process 2");
AUTOSTART_PROCESSES(&hello_world_process2);
/*---------------------------------------------------------------------------*/

typedef struct {
    int temperatura;
    int light_sensor;
    int umidade;
} SensorData;

//light-sensor
static SensorData get_light(void)
{
    int temperatura = ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
    int light = (10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7);
    int umidade = (sht11_sensor.value(SHT11_SENSOR_HUMIDITY));

    temperatura += (rand() % 19) - 8;

    SensorData data = { temperatura, light, umidade };
    return data;
}

static void recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
    char *data = (char *)packetbuf_dataptr();
    int temp, light, umidade;

    char *comma = strchr(data, ',');
    if (comma != NULL) {
        *comma = '\0';
        temp = atoi(data);
        light = atoi(comma + 1);
	umidade = atoi(comma + 2);

       if (temp > -4) {
            temp = -4;
            printf("Ajustando temperatura para -4°C (originalmente %d°C)\n", atoi(data));
        }

        printf("Recebido de %d: Temperatura=%d, Luminosidade=%d, Umidade=%d (HOPS %d)\n",
               originator->u8[0], temp, light, umidade, hops);
    } else {
        printf("Erro ao processar os dados recebidos!\n");
    }
}

static void sensor_avaliator(int temp) {
    leds_off(LEDS_RED); 
    leds_off(LEDS_GREEN);

    if (temp <= 4) {
        leds_on(LEDS_GREEN);
        printf("Temperatura baixa (%d°C) - LED Verde Ligado\n", temp);
    } else {
        leds_on(LEDS_RED);
        printf("Temperatura alta (%d°C) - LED Vermelho Ligado\n", temp);
    }
}

/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(hello_world_process2, ev, data)
{
  static struct etimer periodic;
  static struct etimer et;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(sht11_sensor);  // Ativa o sensor de temperatura e umidade


  collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

  if(linkaddr_node_addr.u8[0] == 1 &&
     linkaddr_node_addr.u8[1] == 0) {
    printf("I am sink\n");
    collect_set_sink(&tc, 1);
  }

  /* Allow some time for the network to settle. */
  etimer_set(&et, 120 * CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));

  while(1) {

    /* Send a packet every 30 seconds. */
    etimer_set(&periodic, CLOCK_SECOND * 30);
    etimer_set(&et, random_rand() % (CLOCK_SECOND * 30));

    PROCESS_WAIT_UNTIL(etimer_expired(&et));

    {
      static linkaddr_t oldparent;
      const linkaddr_t *parent;

      SensorData sensorData = get_light(); // Captura os dados do sensor
    
      printf("Sending: Temp=%d, Light=%d, Umid=%d\n", sensorData.temperatura, sensorData.light_sensor, sensorData.umidade);
    
      printf("Sending\n");
      packetbuf_clear();
      packetbuf_set_datalen(sprintf(packetbuf_dataptr(), "%d,%d", 
                              sensorData.temperatura, 
                              sensorData.light_sensor,
                              sensorData.umidade) + 1);
      collect_send(&tc, 15);
      sensor_avaliator(sensorData.temperatura);

      parent = collect_parent(&tc);
      if(!linkaddr_cmp(parent, &oldparent)) {
        if(!linkaddr_cmp(&oldparent, &linkaddr_null)) {
          printf("#L %d 0\n", oldparent.u8[0]);
        }
        if(!linkaddr_cmp(parent, &linkaddr_null)) {
          printf("#L %d 1\n", parent->u8[0]);
        }
        linkaddr_copy(&oldparent, parent);
      }
    }

    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
