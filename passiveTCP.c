int passivesock(const char *service, const char *transport, int qlen);

/* Процедура passiveTCP - создает пассивный сокет для использования* в сервере TCP */
int
passiveTCP(const char *service, int qlen)
/* Параметры: service - служба, связанная с требуемым портом, qlen - максимальная длина очереди запросов сервера */
{
    return passivesock(service, "tcp", qlen);
}
