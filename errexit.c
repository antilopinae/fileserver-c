#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Процедура errexit - выводит сообщение об ошибке и завершает работу*/
int
errexit(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}
