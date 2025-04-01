#ifndef MQTT_H_
#define MQTT_H_

#include "contiki.h"
#include "net/ip/uip.h"

/* Tipos de eventos MQTT */
typedef enum {
  MQTT_EVENT_CONNECTED = 0,
  MQTT_EVENT_DISCONNECTED,
  MQTT_EVENT_PUBLISH,
  MQTT_EVENT_SUBACK,
  MQTT_EVENT_UNSUBACK,
  MQTT_EVENT_PUBACK
} mqtt_event_t;

/* Níveis de QoS */
typedef enum {
  MQTT_QOS_LEVEL_0 = 0,
  MQTT_QOS_LEVEL_1,
  MQTT_QOS_LEVEL_2
} mqtt_qos_level_t;

/* Opções de retenção */
typedef enum {
  MQTT_RETAIN_OFF = 0,
  MQTT_RETAIN_ON
} mqtt_retain_t;

/* Estados da conexão MQTT */
typedef enum {
  MQTT_CONN_STATE_INITIAL = 0,
  MQTT_CONN_STATE_CONNECTING,
  MQTT_CONN_STATE_CONNECTED,
  MQTT_CONN_STATE_DISCONNECTING
} mqtt_conn_state_t;

/* Estrutura de mensagem MQTT */
struct mqtt_message {
  const char *topic;
  const uint8_t *payload_chunk;
  uint16_t payload_length;
  uint8_t first_chunk;
};

/* Estrutura de conexão MQTT */
struct mqtt_connection {
  struct process *client_process;
  mqtt_conn_state_t state;
  uint8_t auto_reconnect;
  uint8_t out_queue_full;
  uint8_t out_buffer_sent;
  void *user_data;
};

/* Protótipos das funções */
void mqtt_register(struct mqtt_connection *c, struct process *p, const char *id,
                   void (*event)(struct mqtt_connection *, mqtt_event_t, void *),
                   uint16_t max_segment_size);

void mqtt_connect(struct mqtt_connection *c, const char *host, uint16_t port,
                  uint16_t keepalive);

void mqtt_disconnect(struct mqtt_connection *c);

uint8_t mqtt_ready(struct mqtt_connection *c);

void mqtt_set_username_password(struct mqtt_connection *c, const char *username,
                               const char *password);

void mqtt_publish(struct mqtt_connection *c, void *user_data, const char *topic,
                  const uint8_t *payload, uint16_t payload_length,
                  mqtt_qos_level_t qos, mqtt_retain_t retain);

void mqtt_subscribe(struct mqtt_connection *c, void *user_data, const char *topic,
                    mqtt_qos_level_t qos);

void mqtt_unsubscribe(struct mqtt_connection *c, void *user_data, const char *topic);

#endif /* MQTT_H_ */
