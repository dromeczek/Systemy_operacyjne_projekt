#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define K 5
#define MIN_KASY 2
#define MAX_KASY 10

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
    semop(semid, &sb, 1);
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

    data->liczba_klientow = 0;
    data->liczba_kas = MIN_KASY;
    data->alarm_pozarowy = 0;

    while (1) {
        sleep(1);

        sem_p(semid, 0);
        if (data->alarm_pozarowy) {
            printf("\033[0;31mKierownik: Pożar! Zamykam wszystkie kasy.\033[0m\n");
            data->liczba_kas = 0;
            sem_v(semid, 0);
            break;
        }

        int potrzebne_kasy = (data->liczba_klientow / K) + (data->liczba_klientow % K > 0);
        if (potrzebne_kasy < MIN_KASY) potrzebne_kasy = MIN_KASY;
        if (potrzebne_kasy > MAX_KASY) potrzebne_kasy = MAX_KASY;

        data->liczba_kas = potrzebne_kasy;
        printf("Kierownik: Liczba klientów: %d, Liczba kas: %d\n", data->liczba_klientow, data->liczba_kas);
        sem_v(semid, 0);
    }

    // Usuwanie pamięci współdzielonej
    if (shmdt(data) == -1) {
        perror("Błąd odłączania pamięci współdzielonej");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Błąd usuwania pamięci współdzielonej");
    }

    // Usuwanie semaforów
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semaforów");
    }

    printf("Kierownik: Zasoby zostały wyczyszczone.\n");
    return 0;
}
