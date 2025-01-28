#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <time.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define MAX_KASY 10

struct shared_data {
    int liczba_klientow;
    int kolejki[MAX_KASY];
    int otwarte_kasy[MAX_KASY];
};

// Wskaźniki i zmienne globalne dla obsługi sygnału
struct shared_data *data;  // Wskaźnik do pamięci współdzielonej
int semid;                 // ID semafora
int wybrana_kasa = -1;     // Wybrana kasa przez klienta

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

// Funkcja obsługi sygnału SIGTERM
void handle_sigterm(int sig) {
    sem_p(semid, 0);  // Zablokowanie sekcji krytycznej

    if (wybrana_kasa != -1) {
        data->kolejki[wybrana_kasa]--;
        data->liczba_klientow--;
        printf("Klient: Trwa ewakuacja! Opuszczam kolejkę kasy %d. Liczba klientów: %d\n",
               wybrana_kasa + 1, data->liczba_klientow);
    }

    sem_v(semid, 0);  // Odblokowanie sekcji krytycznej

    // Odłączenie pamięci współdzielonej
    shmdt(data);

    exit(0);  // Zakończenie procesu
}

int main() {
    srand(time(NULL) ^ getpid());

    // Ustawienie handlera sygnału SIGTERM
    signal(SIGTERM, handle_sigterm);

    // Podłączenie do pamięci współdzielonej
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

    // Blokowanie dostępu do sekcji krytycznej
    sem_p(semid, 0);

    // Wybór pierwszej otwartej kasy
    while (1) {
        int potencjalna_kasa = rand() % MAX_KASY;
        if (data->otwarte_kasy[potencjalna_kasa]) {
            wybrana_kasa = potencjalna_kasa;
            break;
        }
    }

    // Obsługa przypadku, gdy nie ma otwartych kas
    if (wybrana_kasa == -1) {
        printf("Klient: Nie znaleziono otwartej kasy!\n");
        sem_v(semid, 0);  // Odblokowanie sekcji krytycznej
        shmdt(data);      // Odłączenie pamięci współdzielonej
        return 0;
    }

    // Zwiększenie liczby klientów w wybranej kasie
    data->kolejki[wybrana_kasa]++;
    data->liczba_klientow++;
    printf("Klient: Wchodzę do kolejki kasy %d. Liczba klientów: %d\n", wybrana_kasa + 1, data->liczba_klientow);

    // Odblokowanie dostępu do sekcji krytycznej
    sem_v(semid, 0);

    // Symulacja zakupów (losowe opóźnienie)
   sleep(rand() % 5+6);  // Losowe opóźnienie do 0,5 sekundy

    // Blokowanie dostępu do sekcji krytycznej przed opuszczeniem kolejki
    sem_p(semid, 0);

    // Zmniejszenie liczby klientów w wybranej kasie
    data->kolejki[wybrana_kasa]--;
    data->liczba_klientow--;
    printf("Klient: Wychodzę z kolejki kasy %d. Liczba klientów: %d\n", wybrana_kasa + 1, data->liczba_klientow);

    // Odblokowanie dostępu do sekcji krytycznej
    sem_v(semid, 0);

    // Odłączenie pamięci współdzielonej
    shmdt(data);

    return 0;
}
