#ifndef FILE_IO_H
#define FILE_IO_H

#include "structUser.h"

User *searchUserOnFile(char *path, char *userId);
User *stringToUser(char *userString);
int deleteUserFromFile(char *path, char *userId);
int addUserToFile(char *path, User u);
void printUsers(char *path);
char *userToString(User u);
void erro(char *function, char *method);

#endif /* FILE_IO_H */