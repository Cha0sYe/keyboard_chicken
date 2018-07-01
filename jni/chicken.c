#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 512

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

void send_tap(int fd, int x, int y)
{
    char send_buf[BUFFER_SIZE];
    snprintf(send_buf, BUFFER_SIZE, "d 0 %d %d 0", x, y);
    send(fd, send_buf, sizeof(send_buf), 0);
    snprintf(send_buf, BUFFER_SIZE, "c");
    send(fd, send_buf, sizeof(send_buf), 0);
    usleep(200000);
    snprintf(send_buf, BUFFER_SIZE, "u 0");
    send(fd, send_buf, sizeof(send_buf), 0);
    snprintf(send_buf, BUFFER_SIZE, "c");
    send(fd, send_buf, sizeof(send_buf), 0);
}

int main(int argc, char** argv)
{
    int keys_fd;
    int server_fd = -1;
    struct input_event t;
    char* sockname = "minitouch";
    keys_fd = open(argv[1], O_RDONLY);
    if (keys_fd <= 0)
    {
        fprintf(stderr,"open %s error\n", argv[1]);
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
        read(keys_fd, &t, sizeof(struct input_event));
        if(t.type == 1)
        {
            fprintf(stderr, "key %i state %i \n", t.code, t.value);
            send_tap(server_fd, 600, 1600);
        }
    }
    close(keys_fd);
    return EXIT_SUCCESS;
}
