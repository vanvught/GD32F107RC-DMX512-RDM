$(info $$DEFINES [${DEFINES}])

ifeq ($(findstring NODE_ARTNET,$(DEFINES)),NODE_ARTNET)
	LIBS+=artnet4 artnet e131 uuid
endif

ifeq ($(findstring NODE_E131,$(DEFINES)),NODE_E131)
	ifneq ($(findstring e131,$(LIBS)),e131)
		LIBS+=e131 uuid
	endif
endif

ifeq ($(findstring NODE_OSC_SERVER,$(DEFINES)),NODE_OSC_SERVER)
	LIBS+=oscserver osc
endif

ifeq ($(findstring RDM_CONTROLLER,$(DEFINES)),RDM_CONTROLLER)
	LIBS+=rdmdiscovery
	RDM=1
endif

ifdef RDM
	LIBS+=rdm
endif

ifeq ($(findstring OUTPUT_DMX_SEND,$(DEFINES)),OUTPUT_DMX_SEND)
	LIBS+=dmxsend dmx
endif

ifeq ($(findstring OUTPUT_DMX_PIXEL,$(DEFINES)),OUTPUT_DMX_PIXEL)
	LIBS+=ws28xxdmx ws28xx
endif

LIBS+=remoteconfig spiflashstore spiflash network

ifeq ($(findstring DISPLAY_UDF,$(DEFINES)),DISPLAY_UDF)
	LIBS+=displayudf
endif

LIBS+=properties lightset display hal

$(info $$LIBS [${LIBS}])