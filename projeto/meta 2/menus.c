#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
//#include "structUser.h"
#include "menus.h"
#include "fileIO.h"

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
}

/**
 * Reads and returns any string
 * inserted by the user.
 * 
*/
char *getTextField()
{
    char buf[OPTSIZE];
    char *str = (char *)malloc(1024);
    if (fgets(buf, OPTSIZE, stdin))
    {
        sscanf(buf, "%s", str);
        return str;
    }
    else
        erro("getTextField", "fgets");
}

/**
 * Prints the main menu options.
 * 
*/
void mainMenu()
{
    system("cls || clear"); /* Win & Unix */
    printf("Main Menu\n");
    printf("---------\n");
    char options[2][64] = {"Login", "Sair"};
    for (int i = 0; i < 2; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
    int opt = getOption();

    if (debug)
        printf("Option: %d\nAction: %s\n", opt, options[opt]);

    /*
    Option Handling
    */
}

/**
 * Prints login menu options
*/
void loginMenu()
{
    system("cls || clear"); /* Win & Unix */
    printf("Login Menu\n");
    printf("----------\n");

    printf("Insert your login credentials.\n");
    printf("User ID: ");
    char *userId = getTextField();
    printf("Password: ");
    char *password = getTextField();

    if (debug)
        printf("UserId: %s\nPassword: %s\n", userId, password);

    /*
    Credential Handling
    */
}

/**
 * Prints authorized communications for a given user.
 * 
*/
void authorizedCommsMenu()
{
    system("cls || clear"); /* Win & Unix */
    printf("Authorized Communications Menu\n");
    printf("------------------------------\n");

    char options[4][64] = {"Client-Server", "P2P", "Multicast", "Voltar"};
    for (int i = 0; i < 4; i++)
        printf("> %d - %s\n", i, options[i]);

    printf("Insert an option: ");
    int opt = getOption();

    if (debug)
        printf("Option: %d\nAction: %s\n", opt, options[opt]);

    /*
    Option Handling
    */
}

/**
 * Prints Client-Server communication options.
*/
void clientServerMenu()
{
    system("cls || clear"); /* Win & Unix */
    printf("Client-Server Menu\n");
    printf("------------------\n");
    char options[3][64] = {"Set Destination ID", "Send Message", "Voltar"};
    for (int i = 0; i < 3; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
    int opt = getOption();

    if (debug)
        printf("Option: %d\nAction: %s\n", opt, options[opt]);

    /*
    Option Handling
    */
}

/**
 * Prints P2P communication options.
*/
void p2pMenu()
{
    system("cls || clear"); /* Win & Unix */
    printf("P2P Menu\n");
    printf("------------------\n");
    char options[4][64] = {"Set Destination IP Address", "Set Destination Port", "Send P2P Request", "Voltar"};
    for (int i = 0; i < 4; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
    int opt = getOption();

    if (debug)
        printf("Option: %d\nAction: %s\n", opt, options[opt]);

    /*
    Option Handling
    */
}

/**
 * Prints Multicast communication options.
*/
void multicastMenu()
{
    system("cls || clear"); /* Win & Unix */
    printf("Multicast Menu\n");
    printf("------------------\n");
    printf("Available Multicast Groups\n");
    // TODO: printMulticastGroups - envia request dos grupos assim que este menu é chamado
    printf("--------------------------\n");

    char options[3][64] = {"Create Multicast Group", "Join Multicast Group", "Voltar"};
    for (int i = 0; i < 3; i++)
        printf("> %d - %s\n", i, options[i]);
    printf("Insert an option: ");
    int opt = getOption();

    if (debug)
        printf("Option: %d\nAction: %s\n", opt, options[opt]);

    /*
    Option Handling
    */
}

/*
 * Driver program to test above functions.
 *
*/
int main()
{
    mainMenu();
    loginMenu();
    authorizedCommsMenu();
    clientServerMenu();
    p2pMenu();
    multicastMenu();
}
