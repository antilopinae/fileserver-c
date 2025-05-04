#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <errno.h>

int errexit(const char *format, ...);

unsigned short portbase = 0;
/* Точка отсчета номера порта, для использования в серверах, не работающих с привилегиями пользователя root */

/* Процедура passivesock - распределяет и подключает серверный сокет с использованием протокола TCP или UDP */
int
passivesock(const char *service, const char *transport, int qlen)
/* Параметры: service - служба, связанная с требуемым портом, transport - имя используемого транспортного протокола ("tcp" или "udp"), glen - максимальная длина очереди запросов на подключение к серверу */
{
    struct servent *pse; /* указатель на запись с информацией о службе */
    struct protoent *ppe; /* указатель на запись с информацией о протоколе*/
    struct sockaddr_in sin; /* IP-адрес оконечной точки */
    int s, type; /* Дескриптор сокета и тип сокета */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    /* Преобразовать имя службы в номер порта*/
    if ( pse = getservbyname(service, transport) )
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port)+ portbase);
    else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
        errexit("can't get \"%s\" service entry\n", service);

    /* Преобразовать имя протокола в номер протокола */
    if ( (ppe = getprotobyname(transport)) ==0)
        errexit("can't get \"%s\" protocol entry\n", transport);

    /* Использовать имя протокола для определения типа сокета */
    if (strcmp(transport, "udp") ==0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    /* Распределить сокет */
    s = socket(PF_INET, type, ppe->p_proto);
    if (s <0)
        errexit("can't create socket: %s\n", strerror(errno));

    /* Выполнить привязку сокета */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) <0)
        errexit("can't bind to %s port: %s\n", service,strerror(errno));
    if (type == SOCK_STREAM && listen(s, qlen) <0)
        errexit("can't listen on %s port: %s\n", service,strerror(errno));
    return s;
}
