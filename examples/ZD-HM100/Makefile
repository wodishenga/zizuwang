
CONTIKI_PROJECT = app

all: $(CONTIKI_PROJECT)

MODULES += os/net/app-layer/coap


PROJECT_SOURCEFILES += sht30.c sht30-i2c.c lpm-mode.c 

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

