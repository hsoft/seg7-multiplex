#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <pthread.h>

#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_vcd_file.h>

avr_t * seg7 = NULL;
avr_t * drive = NULL;
avr_vcd_t vcd_file;

int main(int argc, char *argv[])
{
	elf_firmware_t fseg7;
	elf_firmware_t fdrive;

	elf_read_firmware("seg7multiplex.bin", &fseg7);
    fseg7.frequency = 1000000UL;

	elf_read_firmware("drive/drive.bin", &fdrive);
    fdrive.frequency = 1000000UL;

	seg7 = avr_make_mcu_by_name("attiny45");
	if (!seg7) {
		fprintf(stderr, "can't make seg7 MCU\n");
		exit(1);
	}
	avr_init(seg7);
	avr_load_firmware(seg7, &fseg7);

	drive = avr_make_mcu_by_name("attiny45");
	if (!drive) {
		fprintf(stderr, "can't make drive MCU\n");
		exit(1);
	}
	avr_init(drive);
	avr_load_firmware(drive, &fdrive);

	avr_vcd_init(seg7, "gtkwave_output.vcd", &vcd_file, 2000);
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(seg7, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), 1,
		"RCLK");
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(seg7, AVR_IOCTL_IOPORT_GETIRQ('B'), 3), 1,
		"SRCLK");
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(seg7, AVR_IOCTL_IOPORT_GETIRQ('B'), 4), 1,
		"SER_DP");
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(drive, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), 1,
		"INCLK");
	avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(drive, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), 1,
		"INSER");
	printf( "\nLaunching:\n");

    avr_vcd_start(&vcd_file);
	int state = cpu_Running;
    while (seg7->cycle < seg7->frequency) {
        if ((state != cpu_Running) && (state != cpu_Sleeping)) {
            printf("Breaking off early state: %d\n", state);
            break;
        }
		state = avr_run(seg7);
        avr_run(drive);
    }
    avr_vcd_stop(&vcd_file);
}

