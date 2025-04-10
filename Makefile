CONTIKI_PROJECT = sensor_temp_mqtt
APPS+=powertrace

CONTIKI = ../..
CONTIKI_WITH_RIME = 1
#CONTIKI_WITH_IPV6 = 1

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include

