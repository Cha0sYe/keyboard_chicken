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
#define KEYBOARD_EVENT "/dev/input/event9"
#define MOUSE_EVENT "/dev/input/event8"

enum d_pad_dir
{
    DIR_NONE = 0,
    DIR_U,
    DIR_D,
    DIR_L,
    DIR_R,
    DIR_UL,
    DIR_DL,
    DIR_UR,
    DIR_DR
};

static int walk_coordinate[9][2] = {{788, 306}, {670, 306}, {960, 306}, {788, 188},
            {788, 424}, {705, 223}, {871, 223}, {705, 389}, {871, 389}};
static int run_coordinate[9][2] = {{788, 306}, {540, 306}, {960, 306}, {788, 188},
            {788, 424}, {613, 131}, {871, 223}, {613, 481}, {871, 389}};

enum d_pad_dir d_pad_statemachine(int code, int value)
{
    /* wsad_state
     * W S A D
     * 0 0 0 0 DIR_NONE
     * 0 0 0 1 DIR_R
     * 0 0 1 0 DIR_L
     * 0 0 1 1 DIR_NONE
     * 0 1 0 0 DIR_D
     * 0 1 0 1 DIR_DR
     * 0 1 1 0 DIR_DL
     * 0 1 1 1 DIR_D
     * 1 0 0 0 DIR_U
     * 1 0 0 1 DIR_UR
     * 1 0 1 0 DIR_UL
     * 1 0 1 1 DIR_U
     * 1 1 0 0 DIR_NONE
     * 1 1 0 1 DIR_R
     * 1 1 1 0 DIR_L
     * 1 1 1 1 DIR_NONE
     */
    static char wsad_state = 0;
    static enum d_pad_dir current_dir = DIR_NONE;
    static enum d_pad_dir previous_dir = DIR_NONE;
    static enum d_pad_dir conv[16] = {DIR_NONE, DIR_R ,DIR_L, DIR_NONE, DIR_D, DIR_DR, DIR_DL,
                    DIR_D, DIR_U, DIR_UR, DIR_UL, DIR_U, DIR_NONE, DIR_R, DIR_L, DIR_NONE};

    fprintf(stderr, "old wsad_state = %x\n", wsad_state);
    switch(code)
    {
        case KEY_W:
            wsad_state = ((wsad_state & 0x7) | (value << 3));
            break;
        case KEY_S:
            wsad_state = ((wsad_state & 0xB) | (value << 2));
            break;
        case KEY_A:
            wsad_state = ((wsad_state & 0xD) | (value << 1));
            break;
        case KEY_D:
            wsad_state = ((wsad_state & 0xE) | (value << 0));
            break;
        default:
            break;
    }
    fprintf(stderr, "new wsad_state = %x\n", wsad_state);
    previous_dir = current_dir;
    current_dir = conv[wsad_state];
    return current_dir;
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
	usleep(10000);
    snprintf(send_buf, BUFFER_SIZE, "c\r\n");
    send(fd, send_buf, strlen(send_buf), 0);
}

void send_touch_up(int fd, int finger)
{
	char send_buf[BUFFER_SIZE];
    snprintf(send_buf, BUFFER_SIZE, "u %d\r\n", finger);
    send(fd, send_buf, strlen(send_buf), 0);
	usleep(10000);
    snprintf(send_buf, BUFFER_SIZE, "c\r\n");
    send(fd, send_buf, strlen(send_buf), 0);
}

void send_touch_move(int fd, int finger, int x, int y)
{
    char send_buf[BUFFER_SIZE];
    snprintf(send_buf, BUFFER_SIZE, "m %d %d %d 0\r\n", finger, x, y);
    send(fd, send_buf, strlen(send_buf), 0);
	usleep(10000);
    snprintf(send_buf, BUFFER_SIZE, "c\r\n");
    send(fd, send_buf, strlen(send_buf), 0);
}

int main(int argc, char** argv)
{
    int keyboard_fd;
    int server_fd = -1;
    struct input_event t;
    char* sockname = "minitouch";
    keyboard_fd = open(KEYBOARD_EVENT, O_RDONLY);

    static enum d_pad_dir current_dir = DIR_NONE;
    static enum d_pad_dir previous_dir = DIR_NONE;
    static int run_state = 0;

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
			switch(t.code)
            {
                case BTN_LEFT:
                    if(t.value == 1)
                        send_touch_down(server_fd, 1, 555, 132);
                    else
                        send_touch_up(server_fd, 1);
                    break;
                case BTN_RIGHT:
                    if(t.value == 1)
                        send_touch_down(server_fd, 1, 625, 2070);
                    else
                        send_touch_up(server_fd, 1);
                    break;
                case KEY_W:
                case KEY_S:
                case KEY_A:
                case KEY_D:
                case KEY_LEFTSHIFT:
                    run_state = (t.code == KEY_LEFTSHIFT) ? t.value : run_state;
                    previous_dir = current_dir;
                    current_dir = d_pad_statemachine(t.code, t.value);
                    if(previous_dir == current_dir) break;
                    if(previous_dir == DIR_NONE)
                    {
                        send_touch_down(server_fd, 0, walk_coordinate[current_dir][0], walk_coordinate[current_dir][1]);
                        fprintf(stderr, "send touch down\n");
                    } else if(current_dir == DIR_NONE)
                    {
                        send_touch_up(server_fd, 0);
                        fprintf(stderr, "send touch up\n");
                    } else
                    {
                        send_touch_move(server_fd, 0,
                                run_state ? run_coordinate[current_dir][0] : walk_coordinate[current_dir][0],
                                run_state ? run_coordinate[current_dir][1] : walk_coordinate[current_dir][1]);
                        fprintf(stderr, "dir = %d run = %d\n", current_dir, run_state);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    close(keyboard_fd);
    return EXIT_SUCCESS;
}
