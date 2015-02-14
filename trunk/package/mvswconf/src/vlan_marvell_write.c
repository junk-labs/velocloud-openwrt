#include "vlanconfig.h"
#include "mv_ioctl.h"
#include "mv_vlan.h"
#include "mv_dot1x.h"

eth_ports_s eth_port[LAN_TOT_PORTS];
vlan_tbl_s vlan_tbl[VLAN_MAX];

int vlan_port_reg_write(int32_t fd, uint16_t port_num,
                uint16_t location, uint16_t value);
static uint16_t vtu_get_egress_val(int vlanId, int lanPorts);

int vlan_tbl_insert(int port_num, int vlanId)
{
	vlan_tbl[vlanId - 1].lanPorts |= (1 << port_num);
	return VLAN_SUCCESS;
}

/* This function does the following operations
 * 1) Build the VLAN database from ports(which are allowed vlans)
 * 2) Flush the MV vtu database
 * 3) Make an entry in MV vtu for each vlan
 * 4) Write an entry for the lan side sw ports(0,1,2,3)
 */
int vlan_tbl_build()
{
	int port_num;
	int iTotalVlans;
	int index;
	uint16_t egress_flag;	
	int ret = VLAN_SUCCESS;
        int fd;
	uint16_t value;
	char intfname[10];

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
                perror("socket: ");
                exit(-1);
        }

	for(port_num = 0; port_num < LAN_TOT_PORTS; port_num++)
	{
		iTotalVlans = eth_port[port_num].iTotalVlans;
		for(index = 0; index < iTotalVlans; index++)
		{
		  vlan_tbl_insert(port_num,
				  eth_port[port_num].iVlanIds[index]);
		}

		if((eth_port[port_num].portMode == VLAN_TRUNK_MODE) &&
			(eth_port[port_num].iDefaultVlanId))
		{
			vlan_tbl_insert(port_num,
					eth_port[port_num].iDefaultVlanId);
		}
	}			

	/* Flush all VTU in marvell */
	mv_vtu_flush();

	/* Flush all learned MAC */
	for(port_num = 0; port_num < LAN_TOT_PORTS; port_num++)
	{
		sprintf(intfname, "sw%d", port_num);
		mv_mac_flush(intfname, 0);
	}

	for(index = 0; index < VLAN_MAX; index++)
	{
		/* Check if vlan id is configured */
		if(!vlan_tbl[index].lanPorts)
			continue;

		DBG_LOG(INFO, VLAN, "Vlan Id: %d", index + 1);
		egress_flag = vtu_get_egress_val(index + 1, 
				vlan_tbl[index].lanPorts);

		ret = mv_vtu_write(index + 1, 0, 0, 0, egress_flag, 0);
		if(ret < 0)
		{
			DBG_LOG(SEVERE, VLAN, 
				"Marvell: Failed to write vlan id %d", 
				index + 1);
			goto cleanup;
		}
	}
	/* Write to Marvell per port Registers */
	for(port_num = 0; port_num < LAN_TOT_PORTS; port_num++)
        {
		/* Port 7 */
		value = eth_port[port_num].iDefaultVlanId; 
		ret = vlan_port_reg_write(fd, port_num, 0x7, value);
		if(ret < 0)
			goto cleanup;

		/* Port 8 */
		value = 0x2c80;
		ret = vlan_port_reg_write(fd, port_num, 0x8, value);
		if(ret < 0)
			goto cleanup;
	}
			/* End - Marvell per port Registers */

		/* mv_vtu_dump(); */

cleanup:
	shutdown(fd, 2);
	return ret;
}

int vlan_port_reg_write(int32_t fd, uint16_t port_num, 
		uint16_t location, uint16_t value)
{
	int ret;
	ret = mv_reg_write(fd, port_num, 0, location, value);
	if(ret < 0)
	{
		DBG_LOG(SEVERE, VLAN, 
				"Marvell: Failed to per port %x", 
				location);
	}
	return ret;
}

static uint16_t vtu_get_egress_val(int vlanId, int lanPorts)
{
	int index, port_num;
	uint16_t egress_value = 0;
	int len = 0;
	char dbgvalues[50];
	for(index = 0; index < LAN_TOT_PORTS; index++)
	{
		port_num = (1 << index);
		if((lanPorts & port_num) == port_num)
		{
			if(eth_port[index].portMode == VLAN_TRUNK_MODE)
			{
				/* Trunk mode */
				if(eth_port[index].iDefaultVlanId == vlanId)
					egress_value |= (1 << (index * 2));
				else
					egress_value |= (2 << (index * 2));
				len += sprintf(dbgvalues + len, "%d, ", 1);
			}
			else
			{
				/* Access port */
				egress_value |= (1 << (index * 2));
				len += sprintf(dbgvalues + len, "%d, ", 1);
			}
		}
		else
		{
			/* Not a member of vlan */
			egress_value |= (3 << (index * 2));	
			len += sprintf(dbgvalues + len, "%d, ", 0);
		}
	} 
	for(index = 4; index < 6; index++) /* CPU & WAN PORT */
	{
		egress_value |= (2 << (index * 2));
	}

	dbgvalues[len - 2] = '\0';
	DBG_LOG(INFO, VLAN, 
			"LANPORTS = %s %x\n",
			dbgvalues, egress_value);

	return egress_value;
}
