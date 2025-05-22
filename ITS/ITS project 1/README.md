# ITS Projekt 1

- **Autor:** Marek Pazúr (xpazurm00)
- **Datum:** 2025-04-11

## Matice pokrytí artefaktů

Čísla testů jednoznačně identifikují scénář v souborech `.feature`.

| Page                             | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 |
|----------------------------------|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| Book appointment page            | x | x | x | x | x | x | x | x | x |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
| Calendar page                    |   |   |   |   |   |   |   |   |   | x  | x  | x  | x  | x  | x  | x  |    |    |    |    |    |    |    | x  | x  |
| Customers page                   |   |   |   |   |   |   |   |   |   |    |    |    |    |    |    |    | x  | x  | x  | x  | x  | x  | x  |    |    |
| Forms                            | x | x | x | x | x | x | x |   |   | x  | x  |    | x  | x  | x  | x  |    |    |    |    |    |    |    |    | x  |
| Time slots                       |   | x | x |   |   |   |   |   |   |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
| Information validation           |   |   |   | x | x |   |   |   |   |    |    |    |    |    |    |    |    |    |    |    |    |    | x  |    |    |
| Navigation                       | x |   |   |   |   | x | x | x | x |    |    |    |    |    |    |    |    |    |    |    |    | x  |    |    |    |
| Customer list                    |   |   |   |   |   |   |   |   |   |    |    |    |    |    |    | x  | x  | x  | x  | x  | x  |    |    |    |    |
| Customer appointment list        |   |   |   |   |   |   |   |   |   | x  | x  | x  |    |    | x  |    |    |    |    |    |    |    |    |    |    |
| Appointment management window    |   |   |   |   |   |   |   |   |   | x  | x  | x  | x  |    | x  | x  |    |    |    |    |    |    |    |    |    |
| Unavailability management window |   |   |   |   |   |   |   |   |   |    |    |    |    | x  |    |    |    |    |    |    |    |    |    | x  | x  |
| Customer management form         |   |   |   |   |   |   |   |   |   |    |    |    |    |    |    |    | x  | x  | x  |    |    |    | x  |    |    |
| Search                           |   |   |   |   |   |   |   |   |   |    |    |    |    |    |    |    |    |    |    | x  | x  |    |    |    |    |


## Matice pokrytí aktivit

| Activities                               | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 |
|------------------------------------------|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| Selecting date and time                  | x | x | x |   |   |   |   |   |   |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
| Customer information input               |   |   |   | x | x | x | x |   |   |    |    |    |    |    |    |    |    |    |    |    |    |    | x  |    |    |
| Form navigation                          | x |   |   |   |   | x | x | x | x |    |    |    |    |    |    |    |    |    |    |    |    | x  |    |    |    |
| Appointment registration confirm         |   |   |   |   |   |   | x | x |   |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
| Appointment entry management as admin    |   |   |   |   |   |   |   |   |   | x  | x  | x  |    |    | x  | x  |    |    |    |    |    |    |    |    |    |
| Unavailability entry management as admin |   |   |   |   |   |   |   |   |   |    |    |    |    | x  |    |    |    |    |    |    |    |    |    | x  | x  |
| Customer entry management as admin       |   |   |   |   |   |   |   |   |   |    |    |    |    |    | x  | x  | x  | x  | x  |    |    |    |    |    |    |
| Select existing customer entry           |   |   |   |   |   |   |   |   |   |    |    |    | x  |    |    |    |    |    |    |    |    |    |    |    |    |
| Searching customers                      |   |   |   |   |   |   |   |   |   |    |    |    |    |    |    |    |    |    |    | x  | x  |    |    |    |    |
| View customers                           |   |   |   |   |   |   |   |   |   |    |    |    | x  |    |    |    |    |    |    |    |    |    |    |    |    |
| View customer appointments               |   |   |   |   |   |   |   |   |   | x  | x  | x  |    |    | x  |    |    |    |    |    |    |    |    |    |    |


## Matice Feature-Test

| Feature file                          | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 |
|---------------------------------------|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| unregistered_user_reservation.feature | x | x | x | x | x | x | x | x | x |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
| calendar_reservation.feature          |   |   |   |   |   |   |   |   |   | x  | x  | x  | x  | x  | x  | x  |    |    |    |    |    |    |    | x  | x  |
| customers.feature                     |   |   |   |   |   |   |   |   |   |    |    |    |    |    |    |    | x  | x  | x  | x  | x  | x  | x  |    |    |

