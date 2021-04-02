#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <wchar.h>

void menu__config();
void menu__main();
bool menu__pause();
int  menu__select_options(const wchar_t *desc, const wchar_t *opts[]);
void menu__print_options(const wchar_t *desc, const wchar_t *opts[], int num_opts, int line);

#endif // MENU_H
