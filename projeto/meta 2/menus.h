#ifndef MENUS_H
#define MENUS_H

#include "structUser.h"

int getOption();
char *getTextField();
void mainMenu();
void authorizedCommsMenu(User *u);
void clientServerMenu();
void p2pMenu();
void multicastMenu();
void adminMenu();
char *loginMenu();

#endif /* MENUS_H */