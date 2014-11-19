#include <stdlib.h>

typedef enum Command {GET_STATE, SHOOT, GET_MAP, ERR} Command;

Command char_to_command(const char *str, size_t length);
char command_to_char(Command cmd);
