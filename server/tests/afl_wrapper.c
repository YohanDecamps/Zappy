#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(6969);
    addr.sin_addr.s_addr = 0;
    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        write(2, "connect failed\n", 15);
        return 1;
    }

    if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0) {
        write(2, "fcntl failed\n", 13);
        return 1;
    }

    if(fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK) < 0) {
        write(2, "fcntl failed\n", 13);
        return 1;
    }

    ssize_t len;
    char buffer[512];
    /* write(fd, "babr\nFork\n", 10); */
    /* while (len = read(fd, buffer_out, sizeof buffer_out), len > 0) { */
    /*     if (write(1, buffer_out, len) < 0) */
    /*         break; */
    /*     if (len = read(1, buffer_in, sizeof buffer_in), len <= 0) */
    /*         break; */
    /*     if (write(fd, buffer_in, len) < 0) */
    /*         break; */
    /* } */
    while (len = read(0, buffer, sizeof buffer), len >= 0) {
        if (len > 0 && write(fd, buffer, len) < 0)
            break;
        if (len = read(fd, buffer, sizeof buffer), len < 0)
            break;
        if (len > 0 && write(1, buffer, len) < 0)
            break;
    }
    return 0;
}
