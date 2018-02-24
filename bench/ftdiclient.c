#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <ftdi.h>

#include "../common/intmath.h"

#define SER_PIN 0
#define CLK_PIN 1

#define MAX_DIGITS 4
#define SLEEPDELAY 20

struct ftdi_context *g_ftdi = NULL;

static unsigned char ftdi_buf = 0;
static unsigned int value_to_send = 1234;

/* Utils */
static void pinset(char index, bool high)
{
    if (high) {
        ftdi_buf |= 1 << index;
    } else {
        ftdi_buf &= ~(1 << index);
    }
    ftdi_write_data(g_ftdi, &ftdi_buf, 1);
}

static void push_serial(bool high)
{
    pinset(CLK_PIN, false);
    pinset(SER_PIN, high);
    usleep(SLEEPDELAY);
    pinset(CLK_PIN, true);
    usleep(SLEEPDELAY);
}

static void push_digit(uint8_t digit, bool enable_dot)
{
    assert(digit < 10);

    push_serial(digit & 1);
    push_serial(digit & (1 << 1));
    push_serial(digit & (1 << 2));
    push_serial(digit & (1 << 3));
    push_serial(enable_dot);
}

static void push_number(uint32_t val)
{
    int i;

    // we start with an empty CLK to begin;
    push_serial(false);

    for (i = 0; i < MAX_DIGITS; i++) {
        push_digit((val / int_pow10(MAX_DIGITS - i - 1)) % 10, false);
        usleep(SLEEPDELAY);
    }
}


int main()
{
    int ret;

    g_ftdi = ftdi_new();
    ret = ftdi_usb_open(g_ftdi, 0x0403, 0x6014);
    if (ret < 0) {
        fprintf(
            stderr, "unable to open ftdi device: %d (%s)\n",
            ret, ftdi_get_error_string(g_ftdi));
        return 1;
    }
    ftdi_set_bitmode(g_ftdi, 0xff, BITMODE_BITBANG);
    while (1) {
        push_number(value_to_send);
        usleep(1000 * 1000);
        value_to_send++;
    }
    return 0;
}

