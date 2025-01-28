#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define DEFAULT_CLIENTS 5  // Domyślna liczba klientów

void start_client() {
    pid_t pid = fork();

    if (pid == 0) {
        // Proces potomny: uruchomienie programu client
        execl("./client", "./client", NULL);
        // Jeśli execl się nie powiedzie:
        perror("Błąd podczas uruchamiania klienta");
        exit(1);
    } else if (pid < 0) {
        // Obsługa błędu fork
        perror("Błąd fork");
        exit(1);
    }
}

void no_zombie() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));  // Wyzerowanie struktury
    sa.sa_flags = SA_NOCLDWAIT; // Automatyczne "sprzątanie" procesów zombie
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Błąd podczas ustawiania sigaction");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    int num_clients = DEFAULT_CLIENTS;

    // Sprawdzenie, czy użytkownik podał liczbę klientów jako argument
    if (argc > 1) {
        num_clients = atoi(argv[1]);
        if (num_clients <= 0) {
            fprintf(stderr, "Podaj poprawną liczbę klientów (> 0)\n");
            exit(1);
        }
    }

    printf("Generator: Uruchamiam %d klientów...\n", num_clients);

    // Ustawienie automatycznego "sprzątania" procesów zombie
    no_zombie();

    // Uruchamianie klientów
    for (int i = 0; i < num_clients; i++) {
        start_client();
        printf("Generator: Uruchomiono klienta %d\n", i + 1);
       // usleep(100000);  // Opcjonalnie: krótka pauza (100 ms) między tworzeniem klientów
    }

    printf("Generator: Wszyscy klienci uruchomieni.\n");

    // Brak potrzeby oczekiwania na procesy dzieci dzięki SA_NOCLDWAIT

    return 0;
}
