/*
 * This code is derived from the Opteron boards' debug.c.
 * It should go away either there or here, depending what fits better.
 */

static void dump_spd_registers(const struct mem_controller *ctrl)
{
	int i;
	print_debug("\n");
	for(i = 0; i < 4; i++) {
		unsigned device;
		device = ctrl->channel0[i];
		if (device) {
			int j;
			print_debug("dimm: ");
			print_debug_hex8(i);
			print_debug(".0: ");
			print_debug_hex8(device);
			for(j = 0; j < 256; j++) {
				int status;
				unsigned char byte;
				if ((j & 0xf) == 0) {
					print_debug("\n");
					print_debug_hex8(j);
					print_debug(": ");
				}
				status = spd_read_byte(device, j);
				if (status < 0) {
					print_debug("bad device\n");
					break;
				}
				byte = status & 0xff;
				print_debug_hex8(byte);
				print_debug_char(' ');
			}
			print_debug("\n");
		}
		device = ctrl->channel1[i];
		if (device) {
			int j;
			print_debug("dimm: ");
			print_debug_hex8(i);
			print_debug(".1: ");
			print_debug_hex8(device);
			for(j = 0; j < 256; j++) {
				int status;
				unsigned char byte;
				if ((j & 0xf) == 0) {
					print_debug("\n");
					print_debug_hex8(j);
					print_debug(": ");
				}
				status = spd_read_byte(device, j);
				if (status < 0) {
					print_debug("bad device\n");
					break;
				}
				byte = status & 0xff;
				print_debug_hex8(byte);
				print_debug_char(' ');
			}
			print_debug("\n");
		}
	}
}