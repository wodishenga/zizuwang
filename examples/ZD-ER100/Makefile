CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT)
CONTIKI = ../..
# The BR is either native or embedded, and in the latter case must support SLIP
PLATFORMS_EXCLUDE = nrf52dk
# Include RPL BR module
MODULES += os/services/rpl-border-router
PROJECTDIRS += ../../../contiki/protocol
# Include webserver module
MODULES_REL += webserver
# Include optional target-specific module
include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)
PROJECT_SOURCEFILES += protocol.c
PROJECT_SOURCEFILES += md5.c kvconf.c

include $(CONTIKI)/Makefile.include
include Makefile.paho


