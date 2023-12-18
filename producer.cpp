#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <string.h>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>
#include <time.h> 
#include <iomanip>
#include <map>

#define PRODUCER_INDEX 0


using namespace std;

class Producer{
public:
    char name[11];
    double mean;
    double std_dev;
    int T;
    double generate_price(double mean, double std_dev);
};

typedef struct{
    char name[11];
    double price;
} Materials;

int get_nanosec(long nsec){
    while(nsec / 10 != 0 && nsec / 100 != 0){
        nsec = nsec/10;
    }
    return (int)nsec;
}


void print_time(){
    time_t now = time(0);
    tm *ltm = localtime(&now);
    struct timespec start;
    int nsec;
    cerr << '[' << 1 + ltm->tm_mon << '/' << ltm->tm_mday << '/' << 1900 + ltm->tm_year << ' ' << ltm->tm_hour << ':' << ltm->tm_min << ':' << ltm->tm_sec << '.';
    clock_gettime(CLOCK_MONOTONIC, &start);
    nsec = get_nanosec(start.tv_nsec);
    cerr << nsec << "] ";
}

int main(int argc, char** argv){
    int s, n, e;        // s mutex, n signal consumer, e buffer
    Producer producer;
    strcpy(producer.name, argv[1]);
    producer.mean = stod(argv[2]);
    producer.std_dev = stod(argv[3]);
    producer.T = stoi(argv[4]);
    int N = stoi(argv[5]);
    
    // ftok to generate unique key
    // key_t s_key;
    key_t key = ftok("shmfile", 65);

    // INIT s
    s = semget(0x54321, 1, 0666);

    // INIT n
    n = semget (0x54322, 1, 0666);

    // INIT e
    e = semget (0x54323, 1, 0666);

    // shmget returns an identifier in shmid
    int shmid1 = shmget(0x98765, sizeof(Materials) * N, 0666|IPC_CREAT);
    int shmid2 = shmget(0x98766, sizeof(int)*2, 0666|IPC_CREAT);

    struct sembuf sems[1];
    sems[0].sem_num = 0;
    sems[0].sem_flg = 0;    

    double price;
    Materials commodity;
    strcpy(commodity.name, producer.name);

    default_random_engine generator;
    normal_distribution<double> distribution(producer.mean, producer.std_dev);

    while(1){
        double price = distribution(generator);
        commodity.price = price;
        print_time();
        cout << producer.name << ": generating a new value " << price << endl;
        sems[0].sem_op = -1;
        semop(e, sems, 1);
        print_time();
        cout << producer.name << ": trying to get mutex on shared buffer" << endl;
        sems[0].sem_op = -1;
        semop(s, sems, 1);

        //Materials *commodity = (Materials*) shmat(shmid, (void*) 0, 0);
        Materials* buffer = (Materials*)shmat(shmid1, (void*) 0, 0);
        int* indeces = (int*)shmat(shmid2, (void*) 0, 0);
        buffer[indeces[PRODUCER_INDEX]] = commodity;
        indeces[PRODUCER_INDEX] = indeces[PRODUCER_INDEX] % N + 1;
        print_time();
        cout << producer.name << ": placing " << price << " on shared buffer" << endl;

        // detach from shared memory 
        shmdt(buffer);
        shmdt(indeces);

        sems[0].sem_op = 1;
        semop(s, sems, 1);
        sems[0].sem_op = 1;
        semop(n, sems, 1);
        print_time();
        cout << producer.name << ": sleeping for " << producer.T << " ms" << endl;
        this_thread::sleep_for(chrono::milliseconds(producer.T));
    }
    return 0;
}
