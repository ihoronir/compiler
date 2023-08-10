#include "compiler.h"

StringItem new_string_item(char *str) {
    static int num = 0;
    StringItem string_item = checked_malloc(sizeof(*string_item));
    string_item->label = num++;
    string_item->str = str;
    return string_item;
}
