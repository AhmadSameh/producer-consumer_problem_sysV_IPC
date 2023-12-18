#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <random>
#include <map>
#include <vector>
#include <string.h>

#define CONSUMER_INDEX 1

using namespace std;

typedef struct{
    char name[11];
    double price;
} Materials;

void print_table(){
    printf("+-------------------------------------+\n");
    printf("| Currency      |  Price   | AvgPrice |\n");
    printf("+-------------------------------------+\n");
    printf("| ALUMINUM      |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| COPPER        |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| COTTON        |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| CRUDEOIL      |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| GOLD          |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| LEAD          |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| MENTHAOIL     |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| NATURALGAS    |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| NICKEL        |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| SILVER        |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("| ZINC          |    \033[;34m0.00\033[0m  |    \033[;34m0.00\033[0m  |\n");
    printf("+-------------------------------------+\n");
}

void init_materials(map<string, vector<double>> &material_prices){
    material_prices["ALUMINUM"] = {0, 0, 0, 0, 0, 0};
    material_prices["COPPER"] = {1, 0, 0, 0, 0, 0};
    material_prices["COTTON"] = {2, 0, 0, 0, 0, 0};
    material_prices["CRUDEOIL"] = {3, 0, 0, 0, 0, 0};
    material_prices["GOLD"] = {4, 0, 0, 0, 0, 0};
    material_prices["LEAD"] = {5, 0, 0, 0, 0, 0};
    material_prices["MENTHAOIL"] = {6, 0, 0, 0, 0, 0};
    material_prices["NATURALGAS"] = {7, 0, 0, 0, 0, 0};
    material_prices["NICKEL"] = {8, 0, 0, 0, 0, 0};
    material_prices["SILVER"] = {9, 0, 0, 0, 0, 0};
    material_prices["ZINC"] = {10, 0, 0, 0, 0, 0};
}

void update_table(map<string, vector<double>> &material_prices, char* name, double price){
    // PLACE CURSOR AT PRICE LOCATION USING THE MAP
    printf("\033[%d;19H", ((int)material_prices[name][0])+4);
    // UPDATE PRICE
    if(price < material_prices[name][4])
        printf("\033[;31m%7.2lf↓\033[0m", price);
    else if(price > material_prices[name][4])
        printf("\033[;32m%7.2lf↑\033[0m", price);
    else
        printf("\033[;34m%7.2lf\033[0m ", price);
    // GET AVGPRICE
    double new_avg = (material_prices[name][1] + material_prices[name][2] + material_prices[name][3] + material_prices[name][4] + price) / 5.0;
    // PLACE CURSOR AT AVG PRICE LOCATION
    printf("\033[%d;30H", (int)material_prices[name][0]+4);
    // UPDATE AVGPRICE
    if(new_avg < material_prices[name][5])
        printf("\033[;31m%7.2lf↓\033[0m", new_avg);
    else if(new_avg > material_prices[name][5])
        printf("\033[;32m%7.2lf↑\033[0m", new_avg);
    else
        printf("\033[;34m%7.2lf\033[0m ", new_avg);
    // UPDATE MAP VALUES OF THE ELEMENT
    material_prices[name][1] = material_prices[name][2];
    material_prices[name][2] = material_prices[name][3];
    material_prices[name][3] = material_prices[name][4];
    material_prices[name][4] = price;
    material_prices[name][5] = new_avg;
}

int main(int argc, char** argv){
    system("ipcrm --all=shm");
    system("ipcrm --all=sem");
    int N = stoi(argv[1]);

    key_t s_key;
    char buf [N][100];
    int s, n, e;        // s mutex, n signal consumer, e buffer

    union semun {
        int val;                
        struct semid_ds *buf;   
    } sem_attr;

    Materials materials[11];
    
    printf("\e[1;1H\e[2J"); 
    print_table();
    char commodity_name[11];
    double price;

    map<string, vector<double>> material_prices;
    init_materials(material_prices);
    
    // ftok to generate unique key
    key_t key = ftok("shmfile", 65);

    // INIT s
    s = semget(0x54321, 1, 0666 | IPC_CREAT | IPC_EXCL);
    sem_attr.val = 1;
    semctl(s, 0, SETVAL, sem_attr);

    // INIT n
    n = semget (0x54322, 1, 0666 | IPC_CREAT | IPC_EXCL);
    sem_attr.val = 0;
    semctl(n, 0, SETVAL, sem_attr);

    // INIT e
    e = semget (0x54323, 1, 0666 | IPC_CREAT | IPC_EXCL);
    sem_attr.val = N;
    semctl(e, 0, SETVAL, sem_attr);

    // shmget returns an identifier in shmid
    // int shmid = shmget(key, sizeof(Materials) * N + sizeof(int), 0666|IPC_CREAT);
    int shmid1 = shmget(0x98765, sizeof(Materials) * N, 0666|IPC_CREAT);
    int shmid2 = shmget(0x98766, sizeof(int) * 2, 0666|IPC_CREAT);

    struct sembuf sems[1];
    sems[0].sem_num = 0;
    sems[0].sem_flg = 0; 

    while(1){
        sems[0].sem_op = -1;
        semop(n, sems, 1);
        sems[0].sem_op = -1;
        semop(s, sems, 1);

        //Materials* commodity = (Materials*) shmat(shmid, (void*) 0, 0);
        Materials* buffer = (Materials*)shmat(shmid1, (void*) 0, 0);
        int* indeces = (int*)shmat(shmid2, (void*) 0, 0);
        strcpy(commodity_name, buffer[indeces[CONSUMER_INDEX]].name);
        price = buffer[indeces[CONSUMER_INDEX]].price;
        indeces[CONSUMER_INDEX] = indeces[CONSUMER_INDEX] % N + 1;

        //detach from shared memory 
        shmdt(buffer);
        shmdt(indeces);

        sems[0].sem_op = 1; 
        semop(s, sems, 1);
        sems[0].sem_op = 1; 
        semop(e, sems, 1);
        update_table(material_prices, commodity_name, price);
        printf("\n");   
    }

    return 0;
}
