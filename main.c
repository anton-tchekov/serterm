#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>

void *thread_recv(void *arg)
{
	char buf[1024];
	int fd, n;

	fd = *(int *)arg;
	for(;;)
	{
		n = read(fd, buf, sizeof(buf));
		if(n < 0)
		{
			fprintf(stderr, "(read) Error %d: %s\n", errno, strerror(errno));
			exit(1);
		}
		else if(n == 0)
		{
			fprintf(stderr, "(read) Disconnected\n");
			exit(1);
		}

		write(STDOUT_FILENO, buf, n);
	}

	return NULL;
}

void serial_list(void)
{
	DIR *dp;
	struct dirent *ep;
	const char *name;
	int cnt;

	if(!(dp = opendir("/dev/")))
	{
		fprintf(stderr, "(opendir) Error %d: %s\n", errno, strerror(errno));
		return;
	}

	cnt = 0;
	while((ep = readdir(dp)))
	{
		name = ep->d_name;
		if(!strncmp(name, "ttyUSB", 6) ||
			!strncmp(name, "ttyACM", 6))
		{
			++cnt;
			printf("/dev/%s\n", name);
		}
	}

	if(!cnt)
	{
		puts("No USB to serial devices found");
	}

	closedir(dp);
}

int main(int argc, char **argv)
{
	char buf[1024];
	const char *port;
	int n, fd, ret, speed, baud;
	pthread_t thread;
	struct termios tty;

	if(argc != 3)
	{
		fputs("Usage: ./serterm port baud\n\n", stderr);
		serial_list();
		fputc('\n', stderr);
		return 1;
	}

	port = argv[1];
	baud = atoi(argv[2]);

	switch(baud)
	{
		case 300:
			speed = B300;
			break;

		case 600:
			speed = B600;
			break;

		case 1200:
			speed = B1200;
			break;

		case 2400:
			speed = B2400;
			break;

		case 4800:
			speed = B4800;
			break;

		case 9600:
			speed = B9600;
			break;

		case 19200:
			speed = B19200;
			break;

		case 38400:
			speed = B38400;
			break;

		case 57600:
			speed = B57600;
			break;

		case 115200:
			speed = B115200;
			break;

		case 230400:
			speed = B230400;
			break;

		default:
			fprintf(stderr, "Unsupported baud rate\n");
			return 1;
	}

	if((fd = open(port, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
	{
		fprintf(stderr, "(open) Error %d: %s\n", errno, strerror(errno));
		return 1;
	}

	if(tcgetattr(fd, &tty))
	{
		fprintf(stderr, "(tcgetattr) Error %d: %s\n", errno, strerror(errno));
		return 1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN]  = 1;
	tty.c_cc[VTIME] = 0;
	tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
	tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
	tty.c_cflag |= (CLOCAL | CREAD | CS8);
	if(tcsetattr(fd, TCSANOW, &tty))
	{
		fprintf(stderr, "(tcsetattr) Error %d: %s\n", errno, strerror(errno));
		return 1;
	}

	if((ret = pthread_create(&thread, NULL, thread_recv, &fd)))
	{
		fprintf(stderr, "(pthread_create) Error %d: %s\n", ret, strerror(ret));
		return 1;
	}

	for(;;)
	{
		if(!fgets(buf, sizeof(buf), stdin))
		{
			fprintf(stderr, "Error reading!\n");
			continue;
		}

		n = strlen(buf);
		write(fd, buf, n);
	}

	return 0;
}
