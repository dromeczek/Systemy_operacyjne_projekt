Projekt: System Zarządzania Kolejkami w Supermarkecie
Opis projektu
Projekt symuluje działanie systemu zarządzania kolejkami w supermarkecie, wykorzystując pamięć współdzieloną, semafory oraz sygnały w środowisku Linux. Składa się z kilku procesów, które współdziałają w celu zarządzania klientami, kasami oraz obsługą sytuacji awaryjnych (np. pożaru).

Technologie:  
C – język programowania  
System V IPC – komunikacja międzyprocesowa (pamięć współdzielona, semafory)
Unix Signals – obsługa sygnałów
Procesy i synchronizacja – wykorzystanie fork(), shmget(), semget() itp.    
Struktura projektu
Projekt składa się z kilku modułów:  
manager.c – Kierownik supermarketu  
Zarządza liczbą otwartych kas w zależności od liczby klientów.  
Otwiera nowe kasy, jeśli kolejki są zbyt długie.  
Zamkniecie kasy, jeśli klientów jest mniej.  
Obsługuje sytuacje awaryjne, takie jak alarm pożarowy.  
client.c – Klient  
Wybiera pierwszą otwartą kasę i dołącza do jej kolejki.  
Czeka w kolejce, a następnie opuszcza supermarket.  
W przypadku alarmu pożarowego natychmiast opuszcza sklep.  
fireman.c – Strażak  
Symuluje sytuację alarmową (np. pożar).
Wysyła sygnał SIGTERM do wszystkich klientów i zatrzymuje generowanie nowych klientów.
Oczekuje, aż wszyscy klienci opuszczą supermarket, a następnie zamyka wszystkie kasy. 
generator.c – Generator klientów  
Tworzy nowych klientów w losowych odstępach czasu.
Klienci losowo wybierają otwarte kasy i czekają na swoją kolej.  
Mechanizmy działania  
Pamięć współdzielona (shared_data)  

Przechowuje informacje o kolejkach, liczbie klientów i statusie alarmu pożarowego.
Semafory  

Synchronizują dostęp do sekcji krytycznych, zapewniając poprawność operacji na pamięci współdzielonej.
Obsługa sygnałów  

Klient nasłuchuje sygnału SIGTERM, który wymusza jego natychmiastowe opuszczenie kolejki.
Strażak wysyła SIGTERM do klientów i zatrzymuje generator klientów.
