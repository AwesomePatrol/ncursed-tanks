#include "parser.h"

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

Command char_to_command(const char *str, size_t length)
{
    switch (str[0])
    {
        case 'S':
            return GET_STATE;
        case 'M':
            return GET_MAP;
        case 'F':
            return SHOOT;
        default:
            return ERR;
    }
}

char command_to_char(Command cmd)
{
    switch (cmd)
    {
        case GET_STATE:
            return 'S';
        case GET_MAP:
            return 'M';
        case SHOOT:
            return 'F';
        default:
            return '\0';
    }
}
