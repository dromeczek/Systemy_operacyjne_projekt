#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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

    for (int i = 0; i < num_clients; i++) {
        start_client();
        printf("Generator: Uruchomiono klienta %d\n", i + 1);
        usleep(100000);  // Opcjonalnie: krótka pauza (100 ms) między tworzeniem klientów
    }

    // Oczekiwanie na zakończenie wszystkich klientów
    for (int i = 0; i < num_clients; i++) {
        wait(NULL);  // Oczekiwanie na zakończenie jednego procesu klienta
    }

    printf("Generator: Wszyscy klienci zakończyli swoje działanie.\n");

    return 0;
}
