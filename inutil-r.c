#include <stdio.h>
#include <stdlib.h>
#include "inutil-r.h"

table* init_table(char *filename);
void IN_load_table(table* t);
void IN_free_item(item* i);
void IN_free_section(section* s);
void IN_free_table(table* t);

section* IN_find_section(table* t, char* section_name);
item* IN_get_item(section* s, int id);
float IN_get_float(item* i, int idx);
int IN_get_int(item *i, int idx);