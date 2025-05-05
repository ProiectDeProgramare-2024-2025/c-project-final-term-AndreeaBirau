#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct Transaction Transaction;

struct Transaction
{
    char plate[10], zone[10], street[100];
    int hours;
};

const char *valid_zones[] = {"Red", "Green", "Yellow"};

void print_menu()
{
    printf("[\033[92;1mOPPS\033[0m] Here is the list of available commands:\n\n");

    printf("show-zones                                          Shows a list of the available parking zones.\n");
    printf("history     [char[] plate]                          Shows the parking history for a plate number.\n");
    printf("pay         [char[] plate, char[] zone, int hours]  Allows a person to pay parking.\n");
    printf("all-history [char[] plate]                          Shows all the parking history.\n");
}

void show_zones()
{
    printf("[\033[92;1mOPPS\033[0m] Here is the list of the available zones:\n");

    printf("\033[31;1mRed\033[0m Zone       1.50RON/Hour\n");
    printf("\033[32;1mGreen\033[0m Zone     2.00RON/Hour\n");
    printf("\033[93;1mYellow\033[0m Zone    1.00RON/Hour\n");
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

void print_transaction(struct Transaction tr)
{
    printf("# plate: \033[36;1m%s\033[0m | street: \033[33;1m%s\033[0m | zone: %s | hours: \033[95;1m%d\033[0m (price: \033[97;1m%.2f\033[0m) #\n", tr.plate, tr.street, color_zone(tr.zone), tr.hours, tr.hours*price_zone(tr.zone));
}

void update_last_transaction_id()
{
    int id = get_last_transaction_id() + 1;
    FILE *f = fopen("./transactions/_last_transaction_id", "w");
    fprintf(f, "%d", id);
    fclose(f);
}

void pay(int argc, char **argv)
{
    if (argc < 6)
    {
        printf("[\033[31;1mOPPS Error\033[0m] Please insert the plate, the street(with _ instead of spaces), the zone and the amount of hours!\n");
        printf("Example: ./main pay TM25KIJ Strada_Principala Red 7\n");
        return;
    }
    char *plate = *(argv + 2), *street = *(argv+3), *zone = *(argv + 4);
    int hours = atoi(*(argv + 5));

    if (!is_valid_zone(zone))
    {
        printf("[\033[31;1mOPPS Error\033[0m] Invalid zone! Valid zones: Red, Green, Yellow\n");
        return;
    }

    char file_name[50];
    sprintf(file_name, "./transactions/%d.t", get_last_transaction_id() + 1);
    update_last_transaction_id();
    FILE *f = fopen(file_name, "w");
    fprintf(f, "%s;%s;%s;%d;", plate,street, zone, hours);
    fclose(f);

    printf("[\033[92;1mOPPS\033[0m] The parking for \033[36;1m%s\033[0m at \033[33;1m%s\033[0m was paid in zone %s for \033[95;1m%d\033[0m hours (Price: \033[97;1m%.2f\033[0m).\n\n", plate, street, color_zone(zone), hours, hours*price_zone(zone));
}

void parse_transaction(Transaction *tr, char s[150], FILE *f, char t[6][150])
{
    fgets(s, 149, f);
    s[strlen(s) - 1] = '\0';
    int h = 0;

    char *p = strtok(s, ";");
    while (p != NULL)
    {
        strcpy(t[h++], p);
        p = strtok(NULL, ";");
    }

    strcpy(tr->plate, t[0]);
    strcpy(tr->street, t[1]);
    strcpy(tr->zone, t[2]);

    tr->hours = atoi(t[3]);
}

DIR* open_transactions_dir()
{
    DIR *dir = opendir("./transactions");

    readdir(dir);
    readdir(dir);

    return dir;
}

void history(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("[\033[31;1mOPPS Error\033[0m] Please insert the plate! \nExample: ./main history TM25KIJ\n");
        return;
    }

    printf("[\033[92;1mOPPS\033[0m] Here is the history of transactions for plate \"%s\":\n\n", *(argv+2));

    struct dirent *f;
    DIR *dir = open_transactions_dir();

    char s[150], t[6][150];
    Transaction tr;
    while ((f = readdir(dir)) != NULL)
    {
        if (f->d_name[0] == '_')
            continue;

        sprintf(s, "./transactions/%s", f->d_name);
        FILE *ff = fopen(s, "r");
        parse_transaction(&tr, s, ff, t);
        fclose(ff);
        if (strcmp(tr.plate, *(argv+2)) == 0)
            print_transaction(tr);
    }
}

void all_history(int argc, char** argv)
{

    printf("[\033[92;1mOPPS\033[0m] Here is the history of all transactions:\n\n");

    struct dirent *f;
    DIR *dir = open_transactions_dir();

    char s[150], t[6][150];
    Transaction tr;
    while ((f = readdir(dir)) != NULL)
    {
        if (f->d_name[0] == '_')
            continue;

        sprintf(s, "./transactions/%s", f->d_name);
        FILE *ff = fopen(s, "r");
        parse_transaction(&tr, s, ff, t);
        fclose(ff);
        print_transaction(tr);
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
    else if (strcmp(*(argv + 1), "all-history") == 0)
    {
        all_history(argc, argv);
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
        printf("[\033[31;1mBank Error\033[0m] Missing option! To see all available commands, use help!\n");
    }
    return 0;
}