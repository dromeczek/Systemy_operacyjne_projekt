#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define K 5  // Liczba klientów, którą może obsłużyć jedna kasa
#define MIN_KASY 2
#define MAX_KASY 5

struct shared_data {
    int liczba_klientow;
    int kolejki[MAX_KASY];
    int otwarte_kasy[MAX_KASY];
};

void sem_op(int semid, int semnum, int op) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = op;
    sb.sem_flg = 0;
    if (semop(semid, &sb, 1) == -1) {
        perror("Błąd operacji na semaforze");
        exit(1);
    }
}

void sem_p(int semid, int semnum) { sem_op(semid, semnum, -1); }
void sem_v(int semid, int semnum) { sem_op(semid, semnum, 1); }

int main() {
    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Nie można utworzyć pamięci współdzielonej");
        exit(1);
    }

    struct shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Nie można podłączyć pamięci współdzielonej");
        exit(1);
    }

    int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("Nie można utworzyć semafora");
        exit(1);
    }

    semctl(semid, 0, SETVAL, 1);

    // Inicjalizacja pamięci
    data->liczba_klientow = 0;
    for (int i = 0; i < MAX_KASY; i++) {
        data->kolejki[i] = 0;
        data->otwarte_kasy[i] = (i < MIN_KASY) ? 1 : 0;  // Pierwsze 2 kasy otwarte
    }

    while (1) {
        sleep(2);  // Aktualizacja co 2 sekundy

        sem_p(semid, 0);
        printf("\nKierownik: Liczba klientów: %d\n", data->liczba_klientow);

        // Sprawdzenie statusu kas
        int otwarte = 0;
        for (int i = 0; i < MAX_KASY; i++) {
            if (data->otwarte_kasy[i]) {
                printf("Kasa %d: Otwarta, kolejka: %d\n", i, data->kolejki[i]);
                otwarte++;
            }
        }

        // Zamykanie kas, jeśli warunek liczby klientów jest spełniony
        if (data->liczba_klientow < K * (otwarte - 1) && otwarte > MIN_KASY) {
            for (int i = MAX_KASY - 1; i >= 0; i--) {  // Zamykamy kasę z najwyższym numerem
                if (data->otwarte_kasy[i]) {
                    data->otwarte_kasy[i] = 0;
                    printf("Kierownik: Zamykam kasę %d.\n", i);
                    break;
                }
            }
        }

        // Otwieranie kas, jeśli potrzeba
        if (data->liczba_klientow > K * otwarte && otwarte < MAX_KASY) {
            for (int i = 0; i < MAX_KASY; i++) {
                if (!data->otwarte_kasy[i]) {
                    data->otwarte_kasy[i] = 1;
                    printf("Kierownik: Otwieram kasę %d.\n", i);
                    break;
                }
            }
        }

        sem_v(semid, 0);
    }

    return 0;
}
