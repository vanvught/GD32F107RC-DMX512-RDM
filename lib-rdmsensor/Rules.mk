EXTRA_INCLUDES+=../lib-rdm/include ../lib-configstore/include ../lib-properties/include ../lib-device/include ../lib-hal/include

ifneq ($(MAKE_FLAGS),)
	ifneq (,$(findstring CONFIG_STORE_USE_ROM,$(MAKE_FLAGS)))
		EXTRA_INCLUDES+=../lib-flashcode/include
	endif
endif