#include "server.h"

int get_whitespace(const char *str, size_t length)
{
    int i = 0;
    while ( str[i]!=' ' ) i++;
    return i;
}

int cmd_cmp(const char *a, const char *b, size_t end)
{
    int i = 0;
    while ( a[i] == b[i] ) i++;
    if ( i == end ) return 1;
    return 0;
}

Command get_command(const char *str, size_t length)
{
    int cmd_len = get_whitespace(str,length);
    const char cmd[] = "GET_STATE";
    if ( cmd_len == 9 && cmd_cmp(str,cmd,cmd_len) ) return GET_STATE;
}

void parse(const char *buffer, size_t length)
{
    Command command = get_command(*buffer,length);
}
