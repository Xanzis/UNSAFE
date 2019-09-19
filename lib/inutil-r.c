#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inutil-r.h"

// Read modes
#define Sect_name_reading 1
#define Comment 2
#define Line_id_reading 3
#define Quant_reading 4

void inutilerror(char *error_text) {
	printf("Critical error in inutil-r.c\nError message follows:\n");
	printf("%s\n", error_text);
	exit(1);
}

table* init_table(char *filename) {
	table *t_ptr = (table *) malloc(sizeof (table));
	if (!t_ptr) inutilerror("Could not allocate table");

	char *loc = malloc(strlen(filename) + 1);
	strcpy(loc, filename);
	loc[strlen(filename)] = '\0';

	t_ptr->fileloc = loc;

	t_ptr->sectcount = 0;
	t_ptr->sects = NULL;

	return t_ptr;
}

table* IN_load_table(char* fileloc) {
	// Single pass read of a file into a table struct
	FILE *fp = fopen(fileloc, "r");
	if (!fp) inutilerror("Error opening file");

	table *res = init_table(fileloc);

	// Set up reading variables
	int mode = Sect_name_reading;
	int oldmode;
	char buff[256];
	memset(buff, '\0', 256);
	int buffi = 0;
	oldmode = 0;
	section temp_sect = (section) {NULL, 0, NULL};
	item temp_item = (item) {0, 0, NULL};
	int item_idx = 0;
	int quant_idx = 0;

	char c = fgetc(fp);

	while (c != EOF) {
		if (c == '#') {
			// Begin comment. Keep track of the previous mode
			oldmode = mode;
			mode = Comment;
		}
		if (c == '%') {
			// End section. Package up everything, add section to table.
			memset(buff, '\0', 256);
			buffi = 0;

			// Add to the table and reset the section / item
			// The preceding enter character will have triggered an item store and cleanup

			res->sects = (section *) realloc(res->sects, (res->sectcount + 1) * sizeof (section));
			res->sects[res->sectcount] = temp_sect;
			res->sectcount++;

			temp_sect = (section) {NULL, 0, NULL};
			item_idx = 0;


			// Take another char (should be \n) and go to name reading
			c = fgetc(fp);
			if (c == EOF) {
				fclose(fp);
				return res;
			}
			if (c != '\n') inutilerror("'%' Must be followed by newline");
			mode = Sect_name_reading;
			c = fgetc(fp); // Get a new character for the switch block
		}
		switch (mode) {
			case Sect_name_reading:
				if (c == '\n') {
					// Reset the temp_sect and start writing to it
					temp_sect = (section) {NULL, 0, NULL};
					// (The previous pointers aren't being freed because the same memory is referenced
					//    by the section now in the table)

					// Allocate space for the name
					temp_sect.name = (char *) malloc((strlen(buff) + 1) * sizeof (char));
					strcpy(temp_sect.name, buff);
					// Null-terminate the name
					temp_sect.name[strlen(buff)] = '\0';
					// Reset the buffer
					memset(buff, '\0', 256);
					buffi = 0;

					// Section name followed by lines. Mode is now line id reading
					mode = Line_id_reading;
				}

				else {
					buff[buffi] = c;
					buffi++;
					if (buffi >= 255) inutilerror("Buffer length exceeded");
				}
			break;
			case Comment:
				if (c == '\n') {
					mode = oldmode;
				}
			break;
			case Line_id_reading:
				if (c == ' ') {
					// Done with line id. reset temp_item, set id value.
					temp_item = (item) {0, 0, NULL};
					temp_item.id = atoi(buff);
					memset(buff, '\0', 256);
					buffi = 0;

					// Mode is now quant_reading
					mode = Quant_reading;
				}
				else {
					buff[buffi] = c;
					buffi++;
					if (buffi >= 255) inutilerror("Buffer length exceeded");
				}
			break;
			case Quant_reading:
				if (c == ' ') {
					// Done reading value. Determine int/float, reallocate memory in
					//    temp_item.quants, increment quant_idx, clear buffer.
					temp_item.quants = realloc(temp_item.quants, (quant_idx + 1) * sizeof (quant));
					if (strchr(buff, '.')) {
						// strchr returns a null pointer if . is not present in string
						// it's a float
						temp_item.quants[quant_idx] = (quant) {0, atof(buff), 0}; // isint = 0
					}
					else {
						temp_item.quants[quant_idx] = (quant) {atoi(buff), 0, 1}; // isint = 1
					}
					quant_idx++;
					temp_item.quantcount++;

					memset(buff, '\0', 256);
					buffi = 0;
				}
				else if (c == '\n') {
					// Done reading value *and* done reading line. Do all the same steps as for 
					//    end of reading value, but also reallocate memory in the temp_sect,
					//    clear temp_item, and switch mode to Line_id_reading
					temp_item.quants = (quant *) realloc(temp_item.quants, (quant_idx + 1) * sizeof (quant));
					if (strchr(buff, '.')) temp_item.quants[quant_idx] = (quant) {0, atof(buff), 0};
					else temp_item.quants[quant_idx] = (quant) {atoi(buff), 0, 1};
					quant_idx++;
					temp_item.quantcount++;
					memset(buff, '\0', 256);
					buffi = 0;
					// End section copied from above

					// Now finish up the item
					temp_sect.items = (item *) realloc(temp_sect.items, (item_idx + 1) * sizeof(item));
					temp_sect.items[item_idx] = temp_item;
					temp_sect.itemcount++;
					item_idx++;

					temp_item = (item) {0, 0, NULL};
					quant_idx = 0;

					mode = Line_id_reading;
				}
				else {
					// Keep on reading
					buff[buffi] = c;
					buffi++;
					if (buffi >= 255) inutilerror("Buffer length exceeded");
				}
			break;
			default:
				inutilerror("Something somewhere has gone horribly wrong");
		}
		c = fgetc(fp);
	}
	fclose(fp);
	return res;
}

void IN_free_item(item* it) {
	free(it->quants);
}

void IN_free_section(section* s) {
	for (int i = 0; i < s->itemcount; i++) {
		IN_free_item(s->items + i);
	}
	free(s->items);
	free(s->name);
}

void IN_free_table(table* t) {
	for (int i = 0; i < t->sectcount; i++) {
		IN_free_section(t->sects + i);
	}
	free(t->sects);
	free(t->fileloc);
	free(t);
}

section* IN_find_section(table* t, char* section_name) {
	// Returns a pointer to a section within the supplied table with name
	//    matching the (null-terminated) supplied section name.
	for (int i = 0; i < t->sectcount; i++) {
		if (!strcmp(section_name, t->sects[i].name)) {
			// strcmp returns 0 if the strings are equal
			return t->sects + i; // Returns pointer to matching section
		}
	}
	// inutilerror("Requested section name not present in table");
	return NULL;
}

item* IN_get_item(section* s, int id) {
	// Returns a pointer to an item within the given section
	// IDs aren't strictly sequential so it's best to just iterate through
	for (int i = 0; i < s->itemcount; i++) {
		if (s->items[i].id == id) {
			return s->items + i;
		}
	}
	// inutilerror("Requested item id not present in section");
	return NULL;
}

float IN_get_float(item* it, int idx) {
	if (idx >= it->quantcount) {
		fprintf(stderr, "Item ID %d quantcount %d request idx %d\n", it->id, it->quantcount, idx);
		inutilerror("IN_get_float: Requested index overshoots item length");
	}
	if (it->quants[idx].isint) inutilerror("Float requested; integer stored");
	return it->quants[idx].val_float;
}

int IN_get_int(item *it, int idx) {
	if (idx >= it->quantcount) {
		fprintf(stderr, "Item ID %d quantcount %d request idx %d\n", it->id, it->quantcount, idx);
		inutilerror("IN_get_int: Requested index overshoots item length");
	}
	if (!(it->quants[idx].isint)) inutilerror("Integer requested; float stored");
	return it->quants[idx].val_int;
}