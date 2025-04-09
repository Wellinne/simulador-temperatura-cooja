#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include <string.h>

#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "net/netstack.h"

#include <stdio.h>
#include <stdlib.h>

#include "dev/light-sensor.h"
#include "dev/sht11/sht11-sensor.h"

static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process2, "Hello world process 2");
AUTOSTART_PROCESSES(&hello_world_process2);
/*---------------------------------------------------------------------------*/

typedef struct {
    int temperatura;
    int light_sensor;
    int umidade;
} SensorData;

static int base_temp = -18;
static int variation = 0;
static int direction = 1;

int get_temperature() {
    if(rand() % 3 == 0) {
        direction *= -1;
    }
    
    variation += direction;
    variation = variation < 0 ? 0 : (variation > 10 ? 10 : variation);
    
    int final_temp = base_temp + (rand() % (2 * variation + 1)) - variation;
    
    if(rand() % 10 == 0) {
        variation = 0;
        return base_temp;
    }
    
    return final_temp;
}

//light-sensor
static SensorData get_light(void)
{
    int temperatura = get_temperature();
    int light = (10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7);
    int umidade = (sht11_sensor.value(SHT11_SENSOR_HUMIDITY));

    SensorData data = { temperatura, light, umidade };
    return data;
}

static void led_green(void)
{
	leds_on(LEDS_GREEN);
	leds_off(LEDS_BLUE);
	leds_off(LEDS_RED);
}

static void led_blue(void)
{
	leds_off(LEDS_GREEN);
	leds_on(LEDS_BLUE);
	leds_off(LEDS_RED);
}

static void led_red(void)
{
	leds_off(LEDS_GREEN);
	leds_off(LEDS_BLUE);
	leds_on(LEDS_RED);
}

static void sensor_avaliator(int temp){
    if (temp == -18) {
        led_green();
        printf("Temperatura ideal recebida na LED [VERDE]: %d\n", temp);
    } 
    if (temp < -18 && temp >= -30) {
        led_blue();
        printf("Temperatura ok recebida na LED [AZUL]: %d\n", temp);
    } 
    if (temp > -18 && temp <= 10) {
        led_red();
        printf("Temperatura ruim recebida na LED [VERMELHO]: %d\n", temp);
    } 

    printf("\n\n");  
}

static void recv(struct unicast_conn *c, const linkaddr_t *from)
{
    char *data = (char *)packetbuf_dataptr();
    if (data == NULL) {
        printf("Erro: Nenhum dado recebido!\n");
        return;
    }

    if (linkaddr_node_addr.u8[0] == 1 || linkaddr_node_addr.u8[0] == 2) {
        int temp = 0, light = 0, umidade = 0;
        char *token = strtok(data, ",");

        if (token == NULL) goto erro_parse;
        temp = atoi(token);

        token = strtok(NULL, ",");
        if (token == NULL) goto erro_parse;
        light = atoi(token);

        token = strtok(NULL, ",");
        if (token == NULL) goto erro_parse;
        umidade = atoi(token);

        printf("Sink %d.%d recebeu de %d.%d → Temp=%d, Light=%d, Umid=%d\n",
            linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
            from->u8[0], from->u8[1],
            temp, light, umidade);

        sensor_avaliator(temp);

        if (temp != -18) {
	    int ajuste = -18 - temp;
	    char mensagem[30];
	
	    if(ajuste > 0) {
	         sprintf(mensagem, "Aumente %d celsius da temperatura", ajuste);
            } else {
	         sprintf(mensagem, "Diminua %d celsius da temperatura", ajuste);
	    }

	    packetbuf_clear();
	    packetbuf_copyfrom(mensagem, strlen(mensagem) + 1);
	    unicast_send(&uc, from);

	    printf("Comando enviado para %d.%d: %s\n", from->u8[0], from->u8[1], mensagem);
	}

        return;

    erro_parse:
        printf("Erro ao processar os dados recebidos: formato inválido!\n");

    } else {
        printf("Sensor %d.%d recebeu mensagem do sink %d.%d: %s\n",
               linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
               from->u8[0], from->u8[1], data);
    }
}

/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = { recv };
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(hello_world_process2, ev, data)
{
  static struct etimer periodic;
  static struct etimer et;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  unicast_open(&uc, 146, &unicast_callbacks);

  if (linkaddr_node_addr.u8[0] == 1 || linkaddr_node_addr.u8[0] == 2) {
    printf("Eu sou um sink: %d.%d\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
  }

  while(1) {
    etimer_set(&periodic, CLOCK_SECOND * 30);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));

    if (!(linkaddr_node_addr.u8[0] == 1 || linkaddr_node_addr.u8[0] == 2)) {
      SensorData sensorData = get_light();

      linkaddr_t dest;
      if (linkaddr_node_addr.u8[0] <= 3) {
        dest.u8[0] = 1; dest.u8[1] = 0; // envia para sink 1.0
      } else {
        dest.u8[0] = 2; dest.u8[1] = 0; // envia para sink 4.0
      }

      packetbuf_clear();
      packetbuf_set_datalen(sprintf(packetbuf_dataptr(), "%d,%d,%d",
                                     sensorData.temperatura,
                                     sensorData.light_sensor,
                                     sensorData.umidade) + 1);

      unicast_send(&uc, &dest);
      printf("Sensor %d.%d enviou dados para %d.%d\n",
             linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
             dest.u8[0], dest.u8[1]);
    }

    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
