; Autor reseni: marek pazur xpazurm00

; Projekt 2 - INP 2024
; Vigenerova sifra na architekture MIPS64

; DATA SEGMENT
                .data
msg:            .asciiz "marekpazur" ; sem doplnte vase "jmenoprijmeni"
cipher:         .space  31 ; misto pro zapis zasifrovaneho textu
; zde si muzete nadefinovat vlastni promenne ci konstanty,
; napr. hodnoty posuvu pro jednotlive znaky sifrovacho klice
key:            .asciiz "paz" ;

params_sys5:    .space  8 ; misto pro ulozeni adresy pocatku
                          ; retezce pro vypis pomoci syscall 5
                          ; (viz nize "funkce" print_string)

; CODE SEGMENT
                .text

main:
                daddi   r4, r0, msg ;
                ;jal     print_string ; vypis pomoci print_string - viz nize

                daddi r5, r0, cipher ; ADRESA SIFRY
                daddi r6, r0, key; ADRESA KLICE

                ; r8 - r15 temp registry
                daddi r8, r0, 0 ; int i = 0 (POCITADLO KROKU)
                daddi r9, r0, 97 ; int o = 97 (POSUN ASCII)
                daddi r14, r0, 3 ; (mod 3) (VELIKOST KLICE)
                daddi r15, r0, 26 ; (mod 26) (POCET PISMEN ABECEDY)

                ;while(msg[i])
                while: 
                    lb r10, 0(r4) ;  int c = msg[i] 
                    beqz r10, end ;  c == '\0'

                    dsub r10, r10, r9; c = c - 'a'

                    ddiv r8, r14 ; i / 3
                    mfhi r13 ; = i % 3
                    dadd r7, r6, r13 ; key + (i % 3) periodicky posun v poli s ascii hodnotami klicu
                    lb  r13, 0(r7) ; key[i % 3] dereference

                    daddi r13, r13, 1 ; key[i % 3] + 1
                    dsub r13, r13, r9 ; (key[i % 3] + 1) - 'a' CISELNA HODNOTA KLICE

                    daddi r11, r0, 2 ; int x = 2
                    ddiv r8, r11 ; i % 2
                    mfhi r11; x = i % 2 (0 nebo 1)
                    beqz r11, pos_key ; KEY nebo -KEY pro kazdy druhy znak vstupniho retezce

                    dsub r13, r0, r13 ; key = -key Zaporna ciselna hodnota klice, preskoci se pokud je cislo sude v poradi

                    pos_key:

                    ;((c + key) + 26)%26 + 'a' (CYKLICKY POSUV znaku)
                    dadd r10, r10, r13; c = c + key
                    daddi r10, r10, 26; c = c + 26
                    ddiv r10, r15; c / 26
                    mfhi r10 ;  c = c mod 26
                    dadd r10, r10, r9; c = c + 'a'

                    sb r10, 0(r5) ; cipher[i] = ((c + key) + 26)%26 + 'a' (ZAPSANI DO SIFRY)

                    ; INKREMENTACE UKAZATELU
                    daddi r4, r4, 1 ; msg++
                    daddi r5, r5, 1 ; cipher++
                    daddi r8, r8, 1 ; i++
                    j while ;
                end:

                daddi r10, r0, 0;  c = '\0'
                sb r10, 0(r5) ; cipher[N] = '\0' (Pridani ukoncovaciho znaku za konec retezce)

                daddi   r4, r0, cipher; 
                jal     print_string ; vypis sifry pomoci print_string

; NASLEDUJICI KOD NEMODIFIKUJTE!

                syscall 0   ; halt

print_string:   ; adresa retezce se ocekava v r4
                sw      r4, params_sys5(r0)
                daddi   r14, r0, params_sys5    ; adr pro syscall 5 musi do r14
                syscall 5   ; systemova procedura - vypis retezce na terminal
                jr      r31 ; return - r31 je urcen na return address
