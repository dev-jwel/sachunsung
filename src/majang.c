#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "console.h"
#include "menu.h"
#include "util.h"

const char log_file_name[]  = "log.txt";
FILE *log_file;

int main() {
	srand(time(NULL));
	assert((log_file = fopen(log_file_name, "w")) != NULL, "fopen log file");

	console__init();
	menu__main();
	console__fini();

	fclose(log_file);
	return 0;
}
