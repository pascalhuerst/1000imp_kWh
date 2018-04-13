/***
  Copyright (c) 2018 Pascal Huerst

  Authors: Pascal Huerst <pascal.huerst@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
***/


#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>

#define P_SYS_CLASS_GPIO "/sys/class/gpio"
#define N_BUF 64
#define N_RETRIES 5

int export(unsigned int gpio)
{
    int fd, len;
    char buffer[N_BUF];

    fd = open(P_SYS_CLASS_GPIO "/export", O_WRONLY);
    if (fd < 0)
        return fd;

    len = snprintf(buffer, sizeof(buffer), "%d", gpio);
    write(fd, buffer, len);
    close(fd);

    return 0;
}

int unexport(unsigned int gpio)
{
    int fd, len;
    char buffer[N_BUF];

    fd = open(P_SYS_CLASS_GPIO "/unexport", O_WRONLY);
    if (fd < 0)
        return fd;

    len = snprintf(buffer, sizeof(buffer), "%d", gpio);
    write(fd, buffer, len);
    close(fd);
    return 0;
}

enum Dir {
    Input,
    Output
};

int set_dir(unsigned int gpio, enum Dir dir)
{
    int fd;
    char buffer[N_BUF];

    snprintf(buffer, sizeof(buffer), P_SYS_CLASS_GPIO  "/gpio%d/direction", gpio);

    fd = open(buffer, O_WRONLY);
    if (fd < 0)
        return fd;

    if (dir == Output)
        write(fd, "out", 4);
    else if (dir == Input)
        write(fd, "in", 3);

    close(fd);
    return 0;
}

int set_value(unsigned int gpio, unsigned int value)
{
    int fd;
    char buffer[N_BUF];

    snprintf(buffer, sizeof(buffer), P_SYS_CLASS_GPIO "/gpio%d/value", gpio);

    fd = open(buffer, O_WRONLY);
    if (fd < 0)
        return fd;

    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);
    return 0;
}

int get_value(unsigned int gpio, unsigned int *value)
{
    int fd;
    char buffer[N_BUF];
    char ch;

    snprintf(buffer, sizeof(buffer), P_SYS_CLASS_GPIO "/gpio%d/value", gpio);

    fd = open(buffer, O_RDONLY);
    if (fd < 0)
        return fd;

    read(fd, &ch, 1);

    if (ch != '0') {
        *value = 1;
    } else {
        *value = 0;
    }

    close(fd);
    return 0;
}

int set_edge(unsigned int gpio, char *edge)
{
    int fd;
    char buffer[N_BUF];

    snprintf(buffer, sizeof(buffer), P_SYS_CLASS_GPIO "/gpio%d/edge", gpio);

    fd = open(buffer, O_WRONLY);
    if (fd < 0)
        return fd;

    write(fd, edge, strlen(edge) + 1);
    close(fd);
    return 0;
}

int fd_open(unsigned int gpio)
{
    int fd;
    char buffer[N_BUF];

    snprintf(buffer, sizeof(buffer), P_SYS_CLASS_GPIO "/gpio%d/value", gpio);

    fd = open(buffer, O_RDONLY | O_NONBLOCK );

    return fd;
}

int fd_close(int fd)
{
    return close(fd);
}

int main(int argc, char **argv)
{

    const int gpios[] = { 47, 46, 27, 65, 22, 61 };
    const int n_gpios = sizeof(gpios)/sizeof(gpios[0]);

    int fds[n_gpios];
    memset(fds, 0, n_gpios);

    struct pollfd fdset[n_gpios];


    uint64_t pulse_counter[n_gpios];
    int ret, i, retires;
    char *buffer[N_BUF];

    printf("\n");

    for (i=0; i<n_gpios; i++) {

        pulse_counter[i] = 0;

        if ((ret = export(gpios[i]) < 0)) {
            printf("export[%d] returned %d\n", i, ret);
            return -1;
        }
        if ((ret = set_dir(gpios[i], 0)) < 0) {
            printf("set_dir[%d] returned %d\n", i, ret);
            return -1;
        }
        if ((ret = set_edge(gpios[i], "rising")) < 0) {
            printf("set_edge[%d] returned %d\n", i, ret);
            return -1;
        }
        if ((fds[i] = fd_open(gpios[i])) < 0) {
            printf("fd_open[%d] returned %d\n", i, ret);
            return -1;
        }
    }

    retires = N_RETRIES;

    while (1) {

        memset((void*)fdset, 0, sizeof(fdset));

        for (i=0; i<n_gpios; i++) {
            fdset[i].fd = fds[i];
            fdset[i].events = POLLPRI;
        }

        ret = poll(fdset, n_gpios, -1);

        if (ret < 0) {
            printf("poll() returned %d retry %d\n", ret, retires--);
            if (retires)
                continue;
            else
                return ret;
        }

        //Timeout
        //if (ret == 0)

        for (i=0; i<n_gpios; i++) {
            if (fdset[i].revents & POLLPRI) {
                lseek(fdset[i].fd, 0, SEEK_SET);
                ret = read(fdset[i].fd, buffer, N_BUF);
                if (ret)
                    pulse_counter[i]++;
            }
            printf("pulse[%d] + %ld\n", i, pulse_counter[i]);
        }

        printf("\n\n");
        retires = N_RETRIES;
    }

    for (i=0; i<n_gpios; i++) {
        fd_close(fds[i]);
    }

    return 0;
}
