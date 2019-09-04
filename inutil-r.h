#ifndef _INUTIL_
#define _INUTIL_

/*
Structural approach to loading .us files:
Files encode <tables>
tables contain <sections>, which are composed of:
 - a text label
 - a sequence of <items>
items contain:
 - a numerical id
 - a sequence of <quants> (including )
a quant contains:
 - an integer
 - a float
 - a char representing which is the relevant value
 - (this is admittedly gross, space-wise, but it's important for flexibility)
   (and after all all inutil space should be freed pretty early in runtime)

This utility provides these structs as well as a number of helper functions for retrieving values safely.
*/

typedef struct quant quant;
struct quant{
	int val_int;
	float val_float;
	char isint;
};

typedef struct item item;
struct item{
	int id;
	int quantcount;
	quant *quants;
};

typedef struct section section;
struct section {
	char *name;
	int itemcount;
	item *items;
};

typedef struct table table;
struct table {
	char *fileloc;
	int sectcount;
	section *sects;
};

table* init_table(char *filename);
table* IN_load_table(char* fileloc);
void IN_free_item(item* it);
void IN_free_section(section* s);
void IN_free_table(table* t);

section* IN_find_section(table* t, char* section_name);
item* IN_get_item(section* s, int id);
float IN_get_float(item* it, int idx);
int IN_get_int(item *it, int idx);

#endif