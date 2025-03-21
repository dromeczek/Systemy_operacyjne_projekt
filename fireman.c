#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>


#define SHM_KEY 1234
#define SEM_KEY 5678

#define MIN_KASY 2
#define MAX_KASY 10

struct shared_data {
    int liczba_klientow;
    int kolejki[MAX_KASY];
    int otwarte_kasy[MAX_KASY];
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
// Uzyskanie pamięci współdzielonej za pomocą klucza SHM_KEY
    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0666);
    if (shmid == -1) {
        perror("Nie można uzyskać pamięci współdzielonej");
        exit(1);
    }
// Podłączenie pamięci współdzielonej
    struct shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Nie można podłączyć pamięci współdzielonej");
        exit(1);
    }
// Uzyskanie semafora za pomocą klucza SEM_KEY
    int semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("Nie można uzyskać semafora");
        exit(1);
    }
 // Zablokowanie dostępu do pamięci współdzielonej (operacja P)
    sem_p(semid, 0);
// Zgłoszenie alarmu pożarowego
    printf("\033[0;37;41mStrażak: Pożar zgłoszony!\033[0m\n");
// Zakończenie działania generatora klientów
    system("pkill  generator");
// Zakończenie działania klientów
    system("pkill  client");
// Ustawienie flagi alarmu pożarowego w pamięci współdzielonej
    data->alarm_pozarowy = 1;
 // Odblokowanie dostępu do pamięci współdzielonej (operacja V)
    sem_v(semid, 0);

    return 0;
}
