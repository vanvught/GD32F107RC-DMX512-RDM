$(info $$DEFINES [${DEFINES}])

ifeq ($(findstring NO_EMAC,$(DEFINES)),NO_EMAC)
else
	LIBS+=remoteconfig
endif

ifeq ($(findstring NODE_ARTNET,$(DEFINES)),NODE_ARTNET)
	ifeq ($(findstring ARTNET_VERSION=3,$(DEFINES)),ARTNET_VERSION=3)
		LIBS+=artnet
	else
		LIBS+=artnet e131
	endif
endif

ifeq ($(findstring NODE_E131,$(DEFINES)),NODE_E131)
	ifneq ($(findstring e131,$(LIBS)),e131)
		LIBS+=e131
	endif
endif

ifeq ($(findstring NODE_OSC_CLIENT,$(DEFINES)),NODE_OSC_CLIENT)
	LIBS+=osc device
endif

ifeq ($(findstring NODE_OSC_SERVER,$(DEFINES)),NODE_OSC_SERVER)
	LIBS+=osc
endif


ifeq ($(findstring RDM_CONTROLLER,$(DEFINES)),RDM_CONTROLLER)
	RDM=1
	DMX=1
endif

ifeq ($(findstring RDM_RESPONDER,$(DEFINES)),RDM_RESPONDER)
	ifneq ($(findstring NODE_ARTNET,$(DEFINES)),NODE_ARTNET)
		ifneq ($(findstring ,$(LIBS)),)
			LIBS+=
		endif
	endif
	ifneq ($(findstring rdmsensor,$(LIBS)),rdmsensor)
		LIBS+=rdmsensor
	endif
	ifneq ($(findstring rdmsubdevice,$(LIBS)),rdmsubdevice)
		LIBS+=rdmsubdevice
	endif
	RDM=1
	DMX=1
endif

ifeq ($(findstring NODE_RDMNET_LLRP_ONLY,$(DEFINES)),NODE_RDMNET_LLRP_ONLY)
	RDM=1
	ifneq ($(findstring e131,$(LIBS)),e131)
		LIBS+=e131
	endif
	ifneq ($(findstring rdmsensor,$(LIBS)),rdmsensor)
		LIBS+=rdmsensor
	endif
	ifneq ($(findstring rdmsubdevice,$(LIBS)),rdmsubdevice)
		LIBS+=rdmsubdevice
	endif
endif

ifeq ($(findstring OUTPUT_DMX_SEND,$(DEFINES)),OUTPUT_DMX_SEND)
	LIBS+=
	DMX=1
endif

ifdef RDM
	LIBS+=rdm
endif

ifdef DMX
	LIBS+=dmx
endif

ifeq ($(findstring OUTPUT_DMX_PIXEL,$(DEFINES)),OUTPUT_DMX_PIXEL)
	LIBS+=ws28xxdmx ws28xx
endif

ifeq ($(findstring OUTPUT_DMX_PCA9685,$(DEFINES)),OUTPUT_DMX_PCA9685)
	LIBS+=pca9685dmx pca9685
endif

LIBS+=configstore flashcode network

ifeq ($(findstring DISPLAY_UDF,$(DEFINES)),DISPLAY_UDF)
	LIBS+=displayudf
endif

LIBS+=lightset properties display device hal

$(info $$LIBS [${LIBS}])
