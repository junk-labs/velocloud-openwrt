deps_config := \
	../vgasrc/Kconfig \
	/home/jordanr/volume1/dolphin_12012016/SageBios_Mohon_Peak/coreboot/payloads/seabios/src/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
