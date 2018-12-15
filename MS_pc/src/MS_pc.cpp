//============================================================================
// Name        : MS_pc.cpp
// Author      : Diesson
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ctime>

using namespace std;

void serialInit(int fd);

int main() {
	std::list<char> cmdBuffer;

	FILE* fp;
	int  fd;
	int  bytesRead;
	char buffer[32];

	fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);

	fp = fopen("out.csv", "w");
	fclose(fp);

	printf("fd = %d\n", fd);
	if (fd == -1) {
		perror("Couldn't open /dev/ttyACM0");
		return -1;
	}

	serialInit(fd);

	while (1) {
		while ((bytesRead = read(fd, &buffer, 32))) {
			fp = fopen("out.csv", "a+");

			for (int i = 0; i < bytesRead; i++) {

				if(buffer[i] == '\r')
				{
					time_t t = time(0);
					tm* now = localtime(&t);

					sprintf(buffer, " %02d:%02d:%02d;\n", now->tm_hour, now->tm_min, now->tm_sec);
					for (int i = 0; i < 12; i++) {
						cout << buffer[i];
						fprintf(fp, "%c", buffer[i]);
					}
				}else
				{
					cout << buffer[i];
					fprintf(fp, "%c", buffer[i]);
				}

			}
			cout.flush();
			fclose(fp);
		}
	}

	close(fd);

	return 0;
}

void serialInit(int fd) {
	struct termios SerialPortSettings;

	tcgetattr(fd, &SerialPortSettings);

	cfsetispeed(&SerialPortSettings, B9600);
	cfsetospeed(&SerialPortSettings, B9600);

	SerialPortSettings.c_cflag &= ~PARENB;
	SerialPortSettings.c_cflag &= ~CSTOPB;
	SerialPortSettings.c_cflag &= ~CSIZE;
	SerialPortSettings.c_cflag |= CS8;
	SerialPortSettings.c_cflag &= ~CRTSCTS;
	SerialPortSettings.c_cflag |= CREAD | CLOCAL;

	SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY | IGNCR | ICRNL);
	SerialPortSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	SerialPortSettings.c_cc[VMIN]  = 0;
	SerialPortSettings.c_cc[VTIME] = 5;

	if (tcsetattr(fd, TCSANOW, &SerialPortSettings) != 0) {
		printf("Error\n");
	}

	tcflush(fd, TCIFLUSH);
}
