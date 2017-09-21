##
## This file is part of the coreboot project.
##
## Copyright (C) 2014 Sage Electronic Engineering, LLC.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
##

# SPI flash driver interface
ramstage-$(CONFIG_SPI_FLASH) += spi_flash.c

# drivers
ramstage-$(CONFIG_SPI_FLASH_ADESTO) += adesto.c
ramstage-$(CONFIG_SPI_FLASH_AMIC) += amic.c
ramstage-$(CONFIG_SPI_FLASH_ATMEL) += atmel.c
ramstage-$(CONFIG_SPI_FLASH_EON) += eon.c
ramstage-$(CONFIG_SPI_FLASH_MACRONIX) += macronix.c
ramstage-$(CONFIG_SPI_FLASH_MICRON) += micron.c
ramstage-$(CONFIG_SPI_FLASH_SPANSION) += spansion.c
ramstage-$(CONFIG_SPI_FLASH_SST) += sst.c
ramstage-$(CONFIG_SPI_FLASH_STMICRO) += stmicro.c
ramstage-$(CONFIG_SPI_FLASH_WINBOND) += winbond.c
ramstage-$(CONFIG_SPI_FLASH_GIGADEVICE) += gigadevice.c
ramstage-$(CONFIG_SPI_FRAM_RAMTRON) += ramtron.c

ifeq ($(CONFIG_SPI_FLASH_SMM),y)
# SPI flash driver interface
smm-$(CONFIG_SPI_FLASH) += spi_flash.c

# drivers
smm-$(CONFIG_SPI_FLASH_ADESTO) += adesto.c
smm-$(CONFIG_SPI_FLASH_AMIC) += amic.c
smm-$(CONFIG_SPI_FLASH_ATMEL) += atmel.c
smm-$(CONFIG_SPI_FLASH_EON) += eon.c
smm-$(CONFIG_SPI_FLASH_MACRONIX) += macronix.c
smm-$(CONFIG_SPI_FLASH_MICRON) += micron.c
smm-$(CONFIG_SPI_FLASH_SPANSION) += spansion.c
smm-$(CONFIG_SPI_FLASH_SST) += sst.c
smm-$(CONFIG_SPI_FLASH_STMICRO) += stmicro.c
smm-$(CONFIG_SPI_FLASH_WINBOND) += winbond.c
smm-$(CONFIG_SPI_FRAM_RAMTRON) += ramtron.c
smm-$(CONFIG_SPI_FLASH_GIGADEVICE) += gigadevice.c
endif
