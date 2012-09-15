#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>

#include <usb.h>

#include "opendevice.h"

#include "../firmware-8x/requests.h"

#define NR_CHANNELS 8

static struct host_frame {
	uint16_t host_values[NR_CHANNELS];
	uint16_t sample_count;
	uint8_t gain_mask;
} hf;

#define PRODUCT "usb-current-meter-8x"

#define NOMINAL_VOLTAGE 230

static int get_power(usb_dev_handle *handle)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_GET_VALUES,
			      0, 0, (char *)&hf, sizeof(hf), 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}

	//data = le16toh(data);

	return ret;
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
	fprintf(stderr, "Usage: %s -h\n", name);
	fprintf(stderr, "       %s -R\n", name);
	fprintf(stderr, "       %s [options] divisor\n", name);
	fprintf(stderr, "options:\n"
			"  -h         this help\n"
			"  -R         reset device\n"
			"  -b         run in background\n"
			"  -d delay   delay between updates\n"
			"  -r dbname  rrdtool update mode on dbname db\n"
			"  -V voltage scale output for real voltage instead of nominal\n"
			"  divisor    power divisor, correspond to number of turns on the sensor\n"
			);
	exit(1);
}

static void daemonize(void)
{
	int i;
	pid_t pid;

	if ((i = open("/dev/null", O_RDONLY)) != 0) {
		dup2(i, 0);
		close(i);
	}
	if ((i = open("/dev/null", O_WRONLY)) != 1) {
		dup2(i, 1);
		close(i);
	}
	if ((i = open("/dev/null", O_WRONLY)) != 2) {
		dup2(i, 2);
		close(i);
	}

	setsid();

	pid = fork();

	if (pid < 0) {
		perror("fork");
		exit(1);
	} else if (pid) { /* parent */
		exit(0);
	} else { /* child */
	}
}

int main(int argc, char **argv)
{
	FILE * output = stdout;
	usb_dev_handle *handle = NULL;
	int opt;
	char *rrdb = NULL;
	int delay = 500;
	int scale = 1;
	int reset = 0;
	int voltage = 230;
	int background = 0;
	int i;

	usb_init();

	while ((opt = getopt(argc, argv, "bhRr:d:V:")) != -1) {
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
		case 'b':
			background = 1;
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

	if (rrdb) {
		if (background)
			output = popen("rrdtool - > /dev/null 2>&1", "w");
		else
			output = popen("rrdtool -", "w");
		if (!output) {
			perror("popen");
			exit(1);
		}

		if (background)
			daemonize();

		for (;;) {
			get_power(handle);

			fprintf(output, "update %s N", rrdb);
			for (i = 0; i < NR_CHANNELS; i++) {
				fprintf(output, ":%d", hf.host_values[i] * voltage / NOMINAL_VOLTAGE / scale);
			}
			fprintf(output, "\n");

			fflush(output);
			usleep(delay * 1000);
		}
	} else {
		for (;;) {
			get_power(handle);

			for (i = 0; i < NR_CHANNELS; i++) {
				printf("%4d,", hf.host_values[i] * voltage / NOMINAL_VOLTAGE / scale);
			}
			printf("%4d,", hf.sample_count);
			printf("%02x", hf.gain_mask);
			printf("\n");

			fflush(stdout);
			usleep(delay * 1000);
		}
	}

	usb_close(handle);

	return 0;
}
