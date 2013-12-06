# profile for XXX

define Profile/Velocloud
	NAME:=Velocloud
	#PACKAGES:=${VC_PACKAGES} ${WIRELESS}
endef

define Profile/Velocloud/Description
	Build Image for Portwell Dev Board
endef

$(eval $(call Profile,Velocloud))
