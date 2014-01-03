# profile for generic velocloud build;

define Profile/Velocloud
	NAME:=Velocloud
endef

define Profile/Velocloud/Description
	Build Image for Generic Velocloud Distro
endef

$(eval $(call Profile,Velocloud))
