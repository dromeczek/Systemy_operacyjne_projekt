#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define SHM_KEY 1234
#define SEM_KEY 5678

struct shared_data {
    int liczba_klientow;
    int liczba_kas;
    int alarm_pozarowy;
};

void sem_op(int semid, int semnum, int op) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = op;
    sb.sem_flg = 0;
    if (semop(semid, &sb, 1) == -1) { // Trzeci argument to liczba operacji (1 w tym przypadku)
        perror("Błąd podczas semop");
        exit(1);
    }
}


void sem_p(int semid, int semnum) { sem_op(semid, semnum, -1); }
void sem_v(int semid, int semnum) { sem_op(semid, semnum, 1); }

int main() {
    srand(time(NULL) ^ getpid());

    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0666);
    if (shmid == -1) {
        perror("Nie można uzyskać pamięci współdzielonej");
        exit(1);
    }

    struct shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Nie można podłączyć pamięci współdzielonej");
        exit(1);
    }

    int semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("Nie można uzyskać semafora");
        exit(1);
    }

    sem_p(semid, 0);
    if (data->alarm_pozarowy) {
        printf("Klient: Pożar! Uciekam!\n");
        sem_v(semid, 0);
        exit(0);
    }
    data->liczba_klientow++;
    printf("Klient: Wchodzę do sklepu. Liczba klientów: %d\n", data->liczba_klientow);
    sem_v(semid, 0);

    sleep(rand() % 5 + 1);

    sem_p(semid, 0);
    data->liczba_klientow--;
    printf("Klient: Wychodzę ze sklepu. Liczba klientów: %d\n", data->liczba_klientow);
    sem_v(semid, 0);

    return 0;
}
