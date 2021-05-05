#ifndef STRUCT_USER_H
#define STRUCT_USER_H
#define SIZE 64

typedef struct
{
    char userId[SIZE];
    char ipAddr[SIZE];
    char password[SIZE];
    int permissions[3]; // {Client-Server, P2P, Multicast} [0-1]
} User;

#endif /* STRUCT_USER_H */
