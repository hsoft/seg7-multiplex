#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_hex.h>
#include <simavr/sim_vcd_file.h>

avr_t* sim_init(const char *mcu_name, unsigned long freq, const char *hex_fname)
{
	elf_firmware_t f = {{0}};
    ihex_chunk_p chunk = NULL;
    int cnt = read_ihex_chunks(hex_fname, &chunk);
    if (cnt <= 0) {
		fprintf(stderr, "Unable to load IHEX\n");
        return NULL;
    }
    f.flash = chunk[0].data;
    f.flashsize = chunk[0].size;
    f.flashbase = chunk[0].baseaddr;
    printf("Load HEX flash %08x, %d\n", f.flashbase, f.flashsize);
    for (int ci = 1; ci < cnt; ci++) {
        int diff = chunk[ci].baseaddr - f.flashbase;
        int newsize = diff + chunk[ci].size;
        f.flash = realloc(f.flash, newsize);
        memset(f.flash + f.flashsize, 0, newsize - f.flashsize);
        memcpy(f.flash + diff, chunk[ci].data, chunk[ci].size);
        f.flashsize = newsize;
        printf("Merge HEX flash %08x, %d\n", chunk[ci].baseaddr, chunk[ci].size);
    }


	avr_t *mcu = avr_make_mcu_by_name(mcu_name);
	if (!mcu) {
		fprintf(stderr, "AVR '%s' not known\n", mcu_name);
        return NULL;
	}
    f.frequency = freq;
	avr_init(mcu);
	avr_load_firmware(mcu, &f);
    return mcu;
}

int main(int argc, char *argv[])
{
    avr_t * seg7 = sim_init("attiny45", 1000000UL, "seg7multiplex.S.hex");
    avr_t * drive = sim_init("attiny45", 1000000UL, "drive.S.hex");
    if (seg7 == NULL || drive == NULL) {
        return 1;
    }

    // INCLK
    avr_connect_irq(
		avr_io_getirq(drive, AVR_IOCTL_IOPORT_GETIRQ('B'), 0),
		avr_io_getirq(seg7, AVR_IOCTL_IOPORT_GETIRQ('B'), 2)
    );
    // INSER
    avr_connect_irq(
		avr_io_getirq(drive, AVR_IOCTL_IOPORT_GETIRQ('B'), 1),
		avr_io_getirq(seg7, AVR_IOCTL_IOPORT_GETIRQ('B'), 1)
    );

    avr_vcd_t vcd_file;
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
    while (seg7->cycle < seg7->frequency * 2) {
        if ((state != cpu_Running) && (state != cpu_Sleeping)) {
            printf("Breaking off early state: %d\n", state);
            break;
        }
		state = avr_run(seg7);
        avr_run(drive);
    }
    avr_vcd_stop(&vcd_file);
}

