#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <pthread.h>

#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_vcd_file.h>

avr_t * avr = NULL;
avr_vcd_t vcd_file;

int main(int argc, char *argv[])
{
	elf_firmware_t f;
	const char * fname =  "seg7multiplex.bin";

	printf("Firmware pathname is %s\n", fname);
	elf_read_firmware(fname, &f);

	avr = avr_make_mcu_by_name("attiny45");
	if (!avr) {
		fprintf(stderr, "%s: AVR '%s' not known\n", argv[0], f.mmcu);
		exit(1);
	}
	avr_init(avr);
	avr_load_firmware(avr, &f);
    avr->frequency = 1000000UL;

	avr_vcd_init(avr, "gtkwave_output.vcd", &vcd_file, 2000 /* usec */);
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), 1 /* bit */,
		"RCLK");
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 3), 1 /* bit */,
		"SRCLK");
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 4), 1 /* bit */,
		"SER_DP");
	printf( "\nLaunching:\n");

    avr_vcd_start(&vcd_file);
	int state = cpu_Running;
    while (avr->cycle < avr->frequency) {
        if ((state != cpu_Running) && (state != cpu_Sleeping)) {
            printf("Breaking off early state: %d\n", state);
            break;
        }
		state = avr_run(avr);
    }
    avr_vcd_stop(&vcd_file);
}

