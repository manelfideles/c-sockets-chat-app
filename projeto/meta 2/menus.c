#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "structUser.h"
#include "fileIO.h"
#include "menus.h"

#define OPTSIZE 32
int debug = 0;

/**
 * Reads and returns any
 * single-digit number inserted by the user.
 * 
*/
int getOption()
{
    char buf[OPTSIZE];
    int opt;
    if (fgets(buf, OPTSIZE, stdin))
    {
        sscanf(buf, "%d", &opt);
        return opt;
    }
    else
        erro("getOption", "fgets");
    return 0;
}

/**
 * Reads and returns any string
 * inserted by the user.
 * 
*/
char *getTextField()
{
    char buf[1024];
    char *str = (char *)malloc(1024);
    if (fgets(buf, 1024, stdin))
    {
        buf[strcspn(buf, "\n")] = 0;
        strcpy(str, buf);
        return str;
    }
    else
        erro("getTextField", "fgets");
    return NULL;
}

/**
 * Prints the main menu options.
 * 
*/
void mainMenu()
{
    // system("cls || clear"); /* Win & Unix */
    printf("Main Menu\n");
    printf("---------\n");
    char options[2][64] = {"Login", "Quit"};
    for (int i = 0; i < 2; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
}

/**
 * Prints login menu options
 */
char *loginMenu()
{
    // system("cls || clear"); /* Win & Unix */
    char *credentials = (char *)malloc(256);
    printf("Login Menu\n");
    printf("------------------------------\n");

    printf("Insert your login credentials.\n");
    printf("User ID: ");
    char *userId = getTextField();
    printf("Password: ");
    char *password = getTextField();
    sprintf(credentials, "%s %s", userId, password);
    if (debug)
    {
        printf("UserId: %s\nPassword: %s\n", userId, password);
        printf("Credentials: %s\n", credentials);
    }
    return credentials;
}

/**
 * Prints authorized communications for a given user.
 * 
*/
void authorizedCommsMenu(User *u)
{
    // system("cls || clear"); /* Win & Unix */
    printf("Authorized Communications Menu\n");
    printf("------------------------------\n");
    char options[4][64] = {"Client-Server", "P2P", "Multicast", "Logout"};
    for (int i = 0; i < 3; i++)
        if (u->permissions[i])
            printf("> %d - %s\n", i, options[i]);
    printf("> 4 - %s\n", options[3]);
    printf("Insert an option: ");
}

/**
 * Prints Client-Server communication options.
*/
void clientServerMenu()
{
    // system("cls || clear"); /* Win & Unix */
    printf("Client-Server Menu\n");
    printf("------------------------------\n");
    char options[3][64] = {"Set Destination ID", "Send Message", "Return"};
    for (int i = 0; i < 3; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
}

/**
 * Prints Admin controls.
*/
void adminMenu()
{
    // system("cls || clear"); /* Win & Unix */
    printf("Admin Menu\n");
    printf("------------------------------\n");
    printf("Insert a command [ LIST | ADD | DEL | QUIT ]: ");
}

/**
 * Prints P2P communication options.
*/
void p2pMenu()
{
    // system("cls || clear"); /* Win & Unix */
    printf("P2P Menu\n");
    printf("------------------------------\n");
    char options[4][64] = {"Set Destination IP Address", "Set Destination Port", "Send P2P Request", "Return"};
    for (int i = 0; i < 4; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
}

/**
 * Prints Multicast communication options.
*/
void multicastMenu()
{
    // system("cls || clear"); /* Win & Unix */
    printf("Multicast Menu\n");
    printf("------------------------------\n");
    printf("Available Multicast Groups\n");
    // TODO: printMulticastGroups - envia request dos grupos assim que este menu é chamado
    printf("--------------------------\n");
    char options[3][64] = {"Create Multicast Group", "Join Multicast Group", "Return"};
    for (int i = 0; i < 3; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
}

/*
 * Driver program to test above functions.
 *
int main()
{
    mainMenu();
    loginMenu();
    authorizedCommsMenu();
    clientServerMenu();
    p2pMenu();
    multicastMenu();
}
*/