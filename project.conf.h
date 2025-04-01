#ifndef PROJECT_CONF_H
#define PROJECT_CONF_H

// Configurações mínimas para UDP
#define UIP_CONF_UDP 1
#define UIP_CONF_UDP_CONNS 1
#define UIP_CONF_BROADCAST 1

// Reduz tamanho da pilha IP
#define UIP_CONF_BUFFER_SIZE 128

// Desativa TCP completamente
#define UIP_CONF_TCP 0

// Configuração de pilha mínima
#define NETSTACK_CONF_RDC nullrdc_driver

#endif /* PROJECT_CONF_H */
