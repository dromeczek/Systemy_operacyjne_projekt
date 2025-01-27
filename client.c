#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define K 5

struct shared_data {
    int liczba_klientow;
    int kolejki[K];
    int otwarte_kasy[K];
    int alarm_pozarowy;
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

struct shared_data *data;  // Wskaźnik do pamięci współdzielonej
int semid;                // ID semafora
int wybrana_kasa = -1;    // Kasa wybrana przez klienta

// Funkcja obsługująca sygnał SIGTERM
void handle_sigterm(int sig) {
    sem_p(semid, 0);  // Zablokowanie sekcji krytycznej

    if (wybrana_kasa != -1) {
        data->kolejki[wybrana_kasa]--;
        data->liczba_klientow--;
        printf("Klient:Trwa ewakuacja uciekam. Opuszczam kolejkę kasy %d. Liczba klientów: %d\n",
               wybrana_kasa, data->liczba_klientow);
    }

    sem_v(semid, 0);  // Odblokowanie sekcji krytycznej

    // Odłączenie pamięci współdzielonej
    shmdt(data);

    exit(0);  // Zakończenie procesu
}

int main() {
    srand(time(NULL) ^ getpid());

    // Podłączenie pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0666);
    if (shmid == -1) {
        perror("Nie można uzyskać pamięci współdzielonej");
        exit(1);
    }

    data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Nie można podłączyć pamięci współdzielonej");
        exit(1);
    }

    // Uzyskanie semafora
    semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("Nie można uzyskać semafora");
        exit(1);
    }

    // Ustawienie handlera dla sygnału SIGTERM
    signal(SIGTERM, handle_sigterm);

    sem_p(semid, 0);

    // Wybór kasy losowo spośród otwartych
    do {
        wybrana_kasa = rand() % K;  // Losowanie kasy z zakresu 0..K-1
    } while (!data->otwarte_kasy[wybrana_kasa]);

    data->kolejki[wybrana_kasa]++;
    data->liczba_klientow++;
    printf("Klient: Wchodzę do kolejki kasy %d. Liczba klientów: %d\n", wybrana_kasa, data->liczba_klientow);

    sem_v(semid, 0);

    sleep(rand() % 5 + 10);  // Symulacja zakupów

    sem_p(semid, 0);
    data->kolejki[wybrana_kasa]--;
    data->liczba_klientow--;
    printf("Klient: Wychodzę z kolejki kasy %d. Liczba klientów: %d\n", wybrana_kasa, data->liczba_klientow);
    sem_v(semid, 0);

    // Odłączenie pamięci współdzielonej
    shmdt(data);

    return 0;
}
