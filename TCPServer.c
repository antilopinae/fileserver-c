#define _USE_BSD
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>

#define QLEN 32 /* Максимальная длина очереди соединений */
#define BUFSIZE 4096

#include <errno.h>

void reaper(int);
int TCPfile(int fd);
int errexit(const char *format, ...);
int passiveTCP(const char *service, int qlen);

/* Главная процедура - параллельный сервер TCP для службы FILE */
int
main(int argc, char *argv[])
{
    char *service = "file"; /* Имя службы или номер порта */
    struct sockaddr_in fsin; /* Адрес клиента */
    unsigned int alen; /* Длина адреса клиента */
    int msock; /* Ведущий сокет сервера */
    int ssock; /* Ведомый сокет сервера */

    switch (argc) {
    case 1:
        break;
    case 2:
        service = argv[1];
        break;
    default:
        errexit("usage: TCPechod [port]\n");
    }

    msock = passiveTCP(service, QLEN);

    (void) signal(SIGCHLD, reaper);

    while (1) {
        alen = sizeof(fsin);
        ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
        if (ssock <0) {
            if (errno == EINTR)
                continue;
            errexit("accept: %s\n", strerror(errno));
        }
        switch (fork()) {
        case 0: // child
            (void) close(msock);
            exit(TCPfile(ssock));
        default: //parent
            (void) close(ssock);
            break;
        case -1:
            errexit("fork: %s\n", strerror(errno));
        }
    }
}

/* Процедура TCPfile - управляет файлами из метода и тела запроса */
int
TCPfile(int fd)
{
    char buf[BUFSIZE], m[8], p[256];
    char *fl, *b, *cl;
    int f, cc, ci, t = 0;

    while (cc = read(fd, buf+t, BUFSIZE-1-t)) {
        if (cc <0)
            errexit("read: %s\n", strerror(errno));
        t += cc;
        buf[t] = '\0';

        if (t > BUFSIZE - 1)
            errexit("bad payload\n");

        if ( !(b = strstr(buf, "\r\n\r\n")))
            continue;

        if (sscanf(buf, "%7s %255s", m, p) != 2) {
            if (dprintf(fd, "HTTP/1.1 400 Bad Request\r\nConnection: keep-alive\r\n\r\n") <0)
                errexit("bad fd: %s\n", strerror(errno));
            errexit("bad request\n");
        }

        fl = p + 1;

        if (strcmp(m, "GET") == 0) {
            if ((f = open(fl, O_RDONLY)) <0) {
                if (dprintf(fd, "HTTP/1.1 404 Not Found\r\nConnection: keep-alive\r\n\r\n") <0)
                    errexit("bad fd: %s\n", strerror(errno));
            } else {
                if((cc = read(f, buf, BUFSIZE-1)) <0)
                    errexit("bad file: %s\n", strerror(errno));
                (void) close(f);
                buf[cc] = '\0';
                if (dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/plain\r\nConnection: keep-alive\r\n\r\n%s", cc, buf) <0)
                    errexit("bad fd: %s\n", strerror(errno));
            }
        } else if (strcmp(m, "PUT") == 0) {
            cl = strcasestr(buf, "Content-Length: ");
            if (!cl) continue;

            if (sscanf(cl, "Content-Length: %d", &ci) != 1 || ci <= 0)
                errexit("bad length in put request\n");

            b += 4;

            while (t - (b - buf) < ci) {
                if ((cc = read(fd, buf + t, BUFSIZE - t - 1)) <= 0)
                    errexit("read body: %s\n", strerror(errno));
                if((t += cc) > BUFSIZE-1)
                    errexit("buf overloaded\n");
            }

            if((f = open(fl, O_WRONLY | O_CREAT | O_TRUNC, 0644)) <0) {
                if (dprintf(fd, "HTTP/1.1 404 Not Found\r\nConnection: keep-alive\r\n\r\n") <0)
                    errexit("bad fd: %s\n", strerror(errno));
            } else {
                if (write(f, b, ci) != ci) {
                    if (dprintf(fd, "HTTP/1.1 405 Method Not Allowed\r\nConnection: keep-alive\r\n\r\n") <0)
                        errexit("bad fd: %s\n", strerror(errno));
                } else {
                    if (dprintf(fd, "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n\r\n") <0)
                        errexit("bad fd: %s\n", strerror(errno));
                }
                (void) close(f);
            }
        } else if (strcmp(m, "DELETE") == 0) {
            if (unlink(fl) == 0) {
                if (dprintf(fd, "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n\r\n") <0)
                    errexit("bad fd: %s\n", strerror(errno));
            }
            else {
                if (dprintf(fd, "HTTP/1.1 404 Not Found\r\nConnection: keep-alive\r\n\r\n") <0)
                    errexit("bad fd: %s\n", strerror(errno));
            }
        } else {
            if (dprintf(fd, "HTTP/1.1 405 Method Not Allowed\r\nConnection: keep-alive\r\n\r\n") <0)
                errexit("bad fd: %s\n", strerror(errno));
        }

        t = 0;
    }

    (void) close(fd);

    return 0;
}

/* Процедура reaper - убирает записи дочерних процессов-зомби из системных* таблиц */
void
reaper(int sig)
{
    int status;
    while (wait3(&status, WNOHANG, (struct rusage *)0)>= 0)/* Пустое тело цикла */;
}











