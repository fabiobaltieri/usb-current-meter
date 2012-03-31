#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <usb.h>

#include "opendevice.h"

#include "../firmware/requests.h"

#define PRODUCT "usb-current-meter"

#define NOMINAL_VOLTAGE 230

static int get_power(usb_dev_handle *handle)
{
	int ret;
	uint16_t data;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_GET_VALUE,
			      0, 0, (char *)&data, sizeof(data), 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}

	return data;
}

static void set_mode(usb_dev_handle *handle, int mode)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_SET_MODE,
			      mode, 0, NULL, 0, 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}
}

static void send_reset(usb_dev_handle *handle)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_RESET,
			      0, 0, NULL, 0, 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}
}

static void usage(char *name)
{
	fprintf(stderr, "syntax: %s -h\n", name);
	fprintf(stderr, "        %s -R\n", name);
	fprintf(stderr, "        %s [-r dbname] [-d delay] [-a] divisor\n", name);
	exit(1);
}

int main(int argc, char **argv)
{
	usb_dev_handle *handle = NULL;
	int opt;
	char *rrdb = NULL;
	int delay = 100;
	int scale = 1;
	int reset = 0;
	int mode = 0;
	int voltage = 230;

	usb_init();

	while ((opt = getopt(argc, argv, "hRr:d:aV:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			break;
		case 'r':
			rrdb = optarg;
			break;
		case 'R':
			reset = 1;
			break;
		case 'a':
			mode = 1;
			break;
		case 'd':
			delay = strtol(optarg, NULL, 0);
			break;
		case 'V':
			voltage = strtol(optarg, NULL, 0);
			break;
		default:
			usage(argv[0]);
		}
	}

	if (optind < argc)
		scale = strtol(argv[optind++], NULL, 0);

	if (usbOpenDevice(&handle, 0, NULL, 0, PRODUCT, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", PRODUCT);
		exit(1);
	}

	if (reset) {
		send_reset(handle);
		return 0;
	}

	set_mode(handle, mode);

	if (rrdb) {
		for (;;) {
			printf("update %s N:%d\n", rrdb,
					get_power(handle) * voltage / NOMINAL_VOLTAGE / scale);
			fflush(stdout);
			usleep(delay * 1000);
		}
	} else {
		printf("%d\n", get_power(handle) * voltage / NOMINAL_VOLTAGE / scale);
	}

	usb_close(handle);

	return 0;
}
