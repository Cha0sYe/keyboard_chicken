#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 80
#define KEYBOARD_EVENT "/dev/input/event8"
#define MOUSE_EVENT "/dev/input/event9"

int d_pad_statemachine()
{
    return 0;
}

int connect_server(char* sockname)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("creating socket");
        return fd;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(&addr.sun_path[1], sockname, strlen(sockname));

    if (connect(fd, (struct sockaddr *)&addr, sizeof(sa_family_t) + strlen(sockname) + 1) < 0)
    {
        close(fd);
        perror("connecting");
        return -1;
    }
    return fd;
}

void send_touch_down(int fd, int finger, int x, int y)
{
    char send_buf[BUFFER_SIZE];
    snprintf(send_buf, BUFFER_SIZE, "d %d %d %d 0\r\n", finger, x, y);
    send(fd, send_buf, strlen(send_buf), 0);
	fprintf(stderr, "%s\n", send_buf);
	usleep(10000);
    snprintf(send_buf, BUFFER_SIZE, "c\r\n");
    send(fd, send_buf, strlen(send_buf), 0);
	fprintf(stderr, "%s\n", send_buf);
}

void send_touch_up(int fd, int finger)
{
	char send_buf[BUFFER_SIZE];
    snprintf(send_buf, BUFFER_SIZE, "u %d\r\n", finger);
    send(fd, send_buf, strlen(send_buf), 0);
	fprintf(stderr, "%s\n", send_buf);
	usleep(10000);
    snprintf(send_buf, BUFFER_SIZE, "c\r\n");
    send(fd, send_buf, strlen(send_buf), 0);
	fprintf(stderr, "%s\n", send_buf);
}

int main(int argc, char** argv)
{
    int keyboard_fd;
    int server_fd = -1;
    struct input_event t;
    char* sockname = "minitouch";
    keyboard_fd = open(KEYBOARD_EVENT, O_RDONLY);
    if (keyboard_fd <= 0)
    {
        fprintf(stderr,"open %s error\n", KEYBOARD_EVENT);
        return EXIT_FAILURE;
    }
    server_fd = connect_server(sockname);
    if (server_fd < 0)
    {
        fprintf(stderr,"connecting to server failed\n");
        exit(1);
    }
    while (1)
    {
        read(keyboard_fd, &t, sizeof(struct input_event));
        if(t.type == EV_KEY)
        {
			fprintf(stderr, "key %i state %i \n", t.code, t.value);
			if(t.code == KEY_A)
			{
				if(t.value == 1)
					send_touch_down(server_fd, 0, 600, 1600);
				else
					send_touch_up(server_fd, 0);
			}
        }
    }
    close(keyboard_fd);
    return EXIT_SUCCESS;
}
