#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "structUser.h"
#include "fileIO.h"

#define BUFSIZE 256

/**
 * Displays error message and exits.
 * 
 */
void erro(char *function, char *method)
{
    char buf[64];
    sprintf(buf, "\n\n{%s:%s}", function, method);
    perror(buf);
    exit(-1);
}

/**
 * Reads string and outputs a User object.
 * 
 */
User *stringToUser(char *userString)
{
    char id[64], ip[64], pw[64];
    User *u = (User *)malloc(sizeof(User));
    if (sscanf(userString, "%[^,],%[^,],%[^,],%d,%d,%d", id, ip, pw, &(u->permissions[0]), &(u->permissions[1]), &(u->permissions[2])) < 6)
        erro("stringToUser", "sscanf");
    strcpy(u->userId, id);
    strcpy(u->ipAddr, ip);
    strcpy(u->password, pw);
    return u;
}

/**
 * Searches for 'userId' in 'path' file.
 * Returns User on success, NULL if not found.
 * 
*/
User *searchUserOnFile(char *path, char *userId)
{
    char buf[BUFSIZE];
    FILE *fin = fopen(path, "r");
    if (!fin)
        erro("searchUserOnFile", "fopen");

    char line[64];
    while (fgets(buf, BUFSIZE, fin))
    {
        strcpy(line, buf);
        char *ptr = strtok(buf, ",");
        if (!strcmp(userId, ptr))
        {
            if (fclose(fin))
                erro("searchUserOnFile", "fclose");
            return stringToUser(line);
        }
    }
    if (fclose(fin))
        erro("searchUserOnFile", "fclose");
    return NULL;
}

/** 
 * Searches for userId and deletes it from DB.
 * Creates another file to keep all the lines that
 * aren't the one you wish to remove, then deletes
 * the original and renames the new one.
 * Returns 1 on success, 0 on failure.
 * 
 */
int deleteUserFromFile(char *path, char *userId)
{
    char buf[BUFSIZE];
    FILE *fin = fopen(path, "r");
    FILE *fout = fopen("temp.txt", "w");
    if (!fin || !fout)
        erro("deleteUserFromFile", "fopen");

    // copy all userId's (except 'userId') to fout
    char line[64];
    while (fgets(buf, BUFSIZE, fin))
    {
        strcpy(line, buf);
        char *ptr = strtok(buf, ",");
        if (strcmp(userId, ptr))
            fprintf(fout, "%s\n", line);
    }
    if (fclose(fin) || fclose(fout))
        erro("deleteUserFromFile", "fclose");
    if (remove(path))
        erro("deleteUserFromFile", "remove");
    if (rename("temp.txt", path))
        erro("deleteUserFromFile", "rename");
    return 1;
}

/**
 * Equivalent to a typical toString method.
*/
char *userToString(User u)
{
    char *buf = (char *)malloc(BUFSIZE);
    sprintf(buf, "%s,%s,%s,%d,%d,%d\n", u.userId, u.ipAddr, u.password, u.permissions[0], u.permissions[1], u.permissions[2]);
    return buf;
}

/**
 *  Adds user data in the following format:
 * userId,ipAddr,password,permissions[0],permissions[1],permissions[2]
 * 
*/
int addUserToFile(char *path, User u)
{
    User *user;
    FILE *f = fopen(path, "a+");
    if (!f)
        erro("addUserToFile", "fopen");

    if (!(user = searchUserOnFile(path, u.userId)))
    {
        char *userString = userToString(u);
        if (fprintf(f, "%s", userString) < 0)
        {
            if (fclose(f))
                erro("addUserToFile", "fclose");
            erro("addUserToFile", "fprintf");
        }

        printf("User %s added to file %s!\n", u.userId, path);
        if (fclose(f))
            erro("addUserToFile", "fclose");
        free(user);
        free(userString);
        return 1;
    }
    else
    {
        printf("User %s already exists!\n", u.userId);
        if (fclose(f))
            erro("addUserToFile", "fclose");
        free(user);
        return 0;
    }
}

/**
 * Prints all users in DB.
 * 
*/
void printUsers(char *path)
{
    /*
    Prints file contents line-by-line.
    */
    char buffer[BUFSIZE];
    FILE *f = fopen(path, "r");
    if (!f)
        erro("printUsers", "fopen");
    while (fgets(buffer, BUFSIZE, f))
        printf("%s", buffer);
    if (fclose(f))
        erro("printUsers", "fclose");
}

/**
 * Driver program to test above functions.
*/
/* int main()
{
    User u = {
        "fabio",
        "0.0.0.0",
        "fabio",
        {0, 0, 0},
    };
    User u2 = {
        "ruben",
        "1.1.1.1",
        "ruben",
        {1, 1, 1},
    };
    char path[BUFSIZE] = "UserDatabase.txt";
    addUserToFile(path, u);
    addUserToFile(path, u2);

    char *userString = userToString(u);
    printf("%s\n", userString);

    char userString2[BUFSIZE] = "fabio,193.136.129.1,fabio,1,1,1";
    User us = *stringToUser(userString2);
    printf("userId: %s\n", us.userId);
    printf("ipAddr: %s\n", us.ipAddr);
    printf("password: %s\n", us.password);
    printf("permissions: %d, %d, %d\n", us.permissions[0], us.permissions[1], us.permissions[2]);

    printf("--------\n\n");
    User *use = searchUserOnFile(path, u2.userId);
    if (use)
    {
        printf("Before deletion: \n");
        printUsers(path);
        deleteUserFromFile(path, u2.userId);
        printf("After deletion: \n");
        printUsers(path);
    }
    else
        printf("No user found!\n");

    return 0;
} */
