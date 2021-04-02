#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void assert(bool check, char *msg) {
	if( ! check) {
		fprintf(stderr, "%s\n", msg);
		exit(1);
	}
}
