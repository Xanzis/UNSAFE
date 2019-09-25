#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lib/matutil.h"
#include "lib/inutil-r.h"
#include "lib/undefs.h"
#include "lib/visutil-2d.h"

void unsafeerror(char *error_text) {
	printf("Critical error in unsafe.c\nError message follows:\n");
	printf("%s\n", error_text);
	exit(1);
}

int main() {
	return 1;
}