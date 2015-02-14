#include "vlanconfig.h"

#undef VLAN_CFG_FILE
#define VLAN_CFG_FILE "/etc/config/marvell"

#include <jansson.h>

extern eth_ports_s eth_port[LAN_TOT_PORTS];

int parse_edge_config() {
        json_error_t error;
        json_t *JSON_edge_config;
        json_t *JSON_ports;
	int index, total_count;
	int ret = VLAN_SUCCESS;
	const char *json_file = VLAN_CFG_FILE;

        JSON_edge_config = json_load_file(json_file, 0, &error);
        if (JSON_edge_config == NULL) {
                DBG_LOG(SEVERE, VLAN,
				"Error loading config file %s: %d:%d: %s\n",
				json_file, error.line, error.column, error.text);
		ret = VLAN_FAILURE;
		goto cleanup;
        }

        if (json_unpack_ex(JSON_edge_config, &error, 0,
        		   "{s:o}", "ports", &JSON_ports)) {
		DBG_LOG(SEVERE, VLAN,
			"Error parsing json %s: %d:%d: %s\n",
			json_file, error.line, error.column, error.text);
		ret = VLAN_FAILURE;
		goto cleanup;
        }
	total_count = json_array_size(JSON_ports);
	for (index = 0; index < total_count; index++) {
		int port_num = index;
		json_t *JSON_port = json_array_get(JSON_ports, index);
		json_t *JSON_vlanIds = NULL;
		int numVlanIds = 0;
		int defaultVlanId = 0;
		const char *mode = 0;
		int switchId = 0, portId = 0;
		int i;

		if (json_unpack_ex(JSON_port, &error, 0,
				   "{s:o,s:i,s:s,s:i}",
				   "vlanIds", &JSON_vlanIds,
				   "defaultVlanId", &defaultVlanId,
				   "mode", &mode,
				   "switch", &switchId,
				   "switch", &portId)) {
			DBG_LOG(SEVERE, VLAN,
				"Error parsing json %s: %d:%d: %s\n",
				json_file, error.line, error.column, error.text);
			ret = VLAN_FAILURE;
			goto cleanup;
		}

		numVlanIds = json_array_size(JSON_vlanIds);
		if (strcmp(mode, "trunk") == 0) {
			eth_port[port_num].portMode = VLAN_TRUNK_MODE;
		} else {
			eth_port[port_num].portMode = VLAN_ACCESS_MODE;
		}
		eth_port[port_num].iDefaultVlanId = defaultVlanId;
		eth_port[port_num].iTotalVlans = numVlanIds;
		eth_port[port_num].iVlanIds = (int *)calloc(numVlanIds, sizeof(int));
		for (i = 0; i < numVlanIds; i++) {
			eth_port[port_num].iVlanIds[i] =
				json_integer_value(json_array_get(JSON_vlanIds, i));
		}
		eth_port[port_num].iSwitchId = switchId;
		eth_port[port_num].iPort = portId;
	}

cleanup:
	return ret;
}
