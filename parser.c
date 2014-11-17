#include "server.h"

int get_whitespace(const char *str, size_t length)
{
    int i = 0;
    while ( str[i] != ' ' && i < length ) i++;
    return i < length ? i : -1;
}

int cmd_cmp(const char *a, const char *b, size_t end)
{
    int i = 0;
    while ( a[i] == b[i] && i < end ) i++;
    if ( i == end ) return 1;
    return 0;
}

int check_command(const char *str, const char cmd[], size_t cmd_len)
{
    return cmd_len == sizeof(cmd)/sizeof(char)
            && cmd_cmp(str,cmd,cmd_len);
}

Command get_command(const char *str, size_t length)
{
    int cmd_len = get_whitespace(str,length);
    if ( cmd_len == 0 ) return ERR;
    if (check_command(str,"GET_STATE",cmd_len)) return GET_STATE;
    if (check_command(str,"GET_SHOOT",cmd_len)) return SHOOT;
    if (check_command(str,"GET_MAP",cmd_len)) return GET_MAP;
    return ERR;
}

void parse(const char *buffer, size_t length)
{
    Command command = get_command(*buffer,length);
    if ( command == ERR && DEBUG <= 5 ) printf("ERROR: invalid command %s\n",buffer);
}
