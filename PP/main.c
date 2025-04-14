#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
typedef struct Transaction Transaction;
struct Transaction
{
    char plate[10], zone[10];
    int hours;
};
const char *valid_zones[] = {"Red", "Green", "Yellow"};
void print_menu()
{
    printf("[OPPS] Here is the list of available commands:\n\n");
    printf("show-zones                                          Shows a list of the available parking zones.\n");
    printf("history     [char[] plate]                          Shows the parking history for a plate number.\n");
    printf("pay         [char[] plate, char[] zone, int hours]  Allows a person to pay parking.\n");
}
void show_zones()
{
    printf("[OPPS] Here is the list of the available zones:\n");
    printf("\033[31;1mRed\033[0m Zone       1.50RON/Hour\n");
    printf("\033[32;1mGreen\033[0m Zone       2.00RON/Hour\n");
    printf("\033[93;1mYellow\033[0m Zone       1.00RON/Hour\n");
}
const char *color_zone(char *t)
{
    if (strcmp("Red", t) == 0)
    {
        return "\033[31;1mRed\033[0m";
    }
    else if (strcmp("Green", t) == 0)
    {
        return "\033[32;1mGreen\033[0m";
    }
    else if (strcmp("Yellow", t) == 0)
    {
        return "\033[93;1mYellow\033[0m";
    }
    return "ERR";
}
 
float price_zone(char *t)
{
    if (strcmp("Red", t) == 0)
    {
        return 1.50;
    }
    else if (strcmp("Green", t) == 0)
    {
        return 2.00;
    }
    else if (strcmp("Yellow", t) == 0)
    {
        return 1.00;
    }
    return -1;
}
int is_valid_zone(char *t)
{
    for (int i = 0; i < sizeof(valid_zones) / sizeof(valid_zones[0]); i++)
    {
        if (strcmp(valid_zones[i], t) == 0)
        {
            return 1;
        }
    }
    return 0;
}
int get_last_transaction_id()
{
    FILE *f = fopen("./transactions/_last_transaction_id", "r");
    int id = 0;
    if (f != NULL)
    {
        char s[10];
        fgets(s, 10, f);
        fclose(f);
        id = atoi(s);
    } else {
        FILE *f = fopen("./transactions/_last_transaction_id", "w");
        fprintf(f, "1");
    }
    return id;
}
void update_last_account_id()
{
    int id = get_last_transaction_id() + 1;
    FILE *f = fopen("./transactions/_last_transaction_id", "w");
    fprintf(f, "%d", id);
    fclose(f);
}
void pay(int argc, char **argv)
{
    if (argc < 5)
    {
        printf("[OPPS Error] Please insert the plate, the zone and the amount of hours!\n");
        return;
    }
    char *plate = *(argv + 2), *zone = *(argv + 3);
    int hours = atoi(*(argv + 4));
    if (!is_valid_zone(zone))
    {
        printf("[OPPS Error] Invalid zone! Valid zones: Red, Green, Yellow\n");
        return;
    }
    char file_name[50];
    sprintf(file_name, "./transactions/%d.t", get_last_transaction_id() + 1);
    update_last_account_id();
    FILE *f = fopen(file_name, "w");
    fprintf(f, "%s;%s;%d;", plate, zone, hours);
    fclose(f);
    printf("[OPPS] The parking for \033[36;1m%s\033[0m was paid in zone %s for \033[95;1m%d\033[0m hours (Price: \033[97;1m%.2f\033[0m).\n\n", plate, color_zone(zone), hours, hours*price_zone(zone));
}
void parse_transaction(Transaction *tr, char s[50], FILE *f, char t[5][10])
{
    fgets(s, 50, f);
    s[strlen(s) - 1] = '\0';
    int h = 0;
    char *p = strtok(s, ";");
    while (p != NULL)
    {
        strcpy(t[h++], p);
        p = strtok(NULL, ";");
    }
    strcpy(tr->plate, t[0]);
    strcpy(tr->zone, t[1]);
    tr->hours = atoi(t[2]);
}
void history(int argc, char **argv)
{
    printf("[OPPS] Here is the history of the transactions:\n\n");
    struct dirent *f;
    DIR *dir = opendir("./transactions");
    readdir(dir);
    readdir(dir);
    char s[50], t[5][10];
    Transaction tr;
    while ((f = readdir(dir)) != NULL)
    {
        if (f->d_name[0] == '_')
            continue;
        sprintf(s, "./transactions/%s", f->d_name);
        FILE *ff = fopen(s, "r");
        parse_transaction(&tr, s, ff, t);
        fclose(ff);
        printf("# plate: \033[36;1m%s\033[0m | zone: %s | hours: \033[95;1m%d\033[0m (price: \033[97;1m%.2f\033[0m) #\n", tr.plate, color_zone(tr.zone), tr.hours, tr.hours*price_zone(tr.zone));
    }
}

int main(int argc, char **argv)
{
    struct stat st = {0};
    if (stat("./transactions", &st) == -1)
    {
        mkdir("./transactions", 0700);
    }
    system("clear");
    if (argc < 2)
    {
        print_menu();
        return 0;
    }
    if (strcmp(*(argv + 1), "help") == 0)
    {
        print_menu();
    }
    else if (strcmp(*(argv + 1), "show-zones") == 0)
    {
        show_zones();
    }
    else if (strcmp(*(argv + 1), "history") == 0)
    {
        history(argc, argv);
    }
    else if (strcmp(*(argv + 1), "pay") == 0)
    {
        pay(argc, argv);
    }
    else
    {
        printf("[OPPS] Missing option! To see all available commands, use help!\n");
    }
    return 0;
}