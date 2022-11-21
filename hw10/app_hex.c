#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define NOFILL 4
#define BLINK 8

int main(void) {
	int dev, data, rdata;
	
	dev = open("/dev/hex", O_RDWR);
	if (dev < 0) {
		fprintf(stderr, "cannot open LED device\n");
		return 1;
	}

	printf("Input HEX7 data (hex) : ");
	scanf("%x", &data);
	ioctl(dev, 0, NULL);
	write(dev, &data, 4);
	
	read(dev, &rdata, 4);
	printf("read data = %x\n", rdata);

	printf("Input HEX7 data (hex) for BLINK : ");
	scanf("%x", &data);
	ioctl(dev, BLINK, NULL);
	write(dev, &data, 4);

	printf("Input HEX7 data (hex) for NOFILL : ");
	scanf("%x", &data);
	ioctl(dev, NOFILL, NULL);
	write(dev, &data, 4);

	printf("Input HEX7 data (hex) for NOFILL, BLINK : ");
	scanf("%x", &data); 
	ioctl(dev, NOFILL | BLINK, NULL);
	write(dev, &data, 4);
	
	close(dev);
	
	return 0;
}

