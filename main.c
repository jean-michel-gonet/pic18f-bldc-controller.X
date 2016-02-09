/*
 * File:   pic18f-bldc-trp.c
 * Auteur: jmgonet
 *
 * Créé Mars 29, 2015, 10:21 AM
 */
#include <htc.h>
#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "domaine.h"
#include "file.h"
#include "tableauDeBord.h"
#include "puissance.h"
#include "moteur.h"

/**
 * Bits de configuration:
 */
#pragma config FOSC = INTIO67  // Oscillateur interne, ports A6 et A7 comme IO.
#pragma config IESO = OFF      // Pas d'embrouilles avec l'osc. au démarrage.
#pragma config FCMEN = OFF     // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR // RE3 est actif comme master reset.
#pragma config WDTEN = OFF     // Watchdog inactif (pour ICSP /ICD)
#pragma config LVP = OFF       // Single Supply Enable bits off.

// Configure les sorties des PWM B et C pour ne pas interférer entre eux
#pragma config CCP2MX = PORTC1  // P2A sur PORTC1
#pragma config CCP3MX = PORTC6  // P3A sur PORTC6

#ifndef TEST

#define TEMPS_BASE_DE_TEMPS 2656

/**
 * Routine de traitement d'interruptions de basse priorité.
 * Pilotage du moteur sur la base des détecteurs Hall.
 */
void low_priority interrupt interruptionsBPTest() {
    unsigned char hall;
    static unsigned char hall0 = 0;
    static int tempsMesureVitesse = TEMPS_BASE_DE_TEMPS;
    unsigned char potentiometre;

    // Traitement des conversion AD:
    if (PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0;

        if (!ADCON0bits.GODONE) {
            potentiometre = ADRESH - 128;
            enfileEvenement(LECTURE_POTENTIOMETRE, potentiometre);
            ADCON0bits.GODONE = 1;
        }
    }

    // Traitement pour le moteur:
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;

        // Événement base de temps:
        if (-- tempsMesureVitesse == 0) {
            enfileEvenement(BASE_DE_TEMPS, 0);
            tempsMesureVitesse = TEMPS_BASE_DE_TEMPS;
        }

        // Événement PHASE:
        hall = PORTA & 7;
        if (hall != hall0) {
            enfileEvenement(MOTEUR_PHASE, hall);
            hall0 = hall;
        }
    }
}

/**
 * Point d'entrée.
 * Active les PWM 1 à 3, en mode Demi-pont, pour produire un PWM
 * à 62KHz, avec une précision de 1024 pas.
 */
void main() {
    struct EVENEMENT_ET_VALEUR *ev;

    // Configure tous les ports comme entrées:
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xFF;

    
    // Configure le micro contrôleur pour 64MHz:
    OSCCONbits.IRCF = 7;    // Fréquence de base: 16 MHz
    OSCTUNEbits.PLLEN = 1;  // Active le PLL.

    // Configure le module A/D:
    ANSELA = 0x00;       // Désactive les convertisseurs A/D
    ANSELB = 0b00000100; // Active AN8 / RB2 comme entrée analogique.
    ANSELC = 0x00;       // Désactive les convertisseurs A/D.

    ADCON2bits.ADFM = 0; // Résultat justifié sur ADRESH.
    ADCON2bits.ACQT = 5; // Temps d'acquisition: 12 TAD
    ADCON2bits.ADCS = 6; // TAD de 1uS pour FOSC = 64MHz

    ADCON0bits.CHS = 8;  // Canal AN8 (RB2).
    ADCON0bits.ADON = 1; // Active le module A/D.

    // Active le temporisateur 1 (pour piloter les conversions A/D):
    T1CONbits.TMR1ON = 1;   // Active le temporisateur 1
    T1CONbits.TMR1CS = 0;   // Source: FOSC / 4
    T1CONbits.T1CKPS = 0;   // Pas de division de fréquence.
    T1CONbits.T1RD16 = 1;   // Temporisateur de 16 bits.

    // Active le temporisateur 2 (pour gérer les PWM):
    T2CONbits.TMR2ON = 1;
    T2CONbits.T2CKPS = 1;   // Pas de division de fréquence.
    T2CONbits.T2OUTPS = 0;  // Pas de division de fréquence.
    PR2 = 255;              // Période max: 64MHz / (4 * 255) = 62kHz.

    // Active les CCP 1 à 3, tous sur le TMR2:
    CCP1CONbits.CCP1M = 12;         // Sorties P1A, P1B actives à niveau haut.
    CCP1CONbits.P1M = 0;            // Contrôleur de demi pont (P1A et P1B).
    PWM1CONbits.P1DC = TEMPS_MORT;  // Temps mort entre sorties complémentaires.
    CCPTMRS0bits.C1TSEL = 0;        // Utilise TMR2.

    CCP2CONbits.CCP2M = 12;         // Sorties P2A et P2B actives à niveau haut.
    CCP2CONbits.P2M1 = 0;           // Contrôleur de demi pont (P2A et P2B).
    PWM2CONbits.P2DC = TEMPS_MORT;  // Temps mort entre sorties complémentaires.
    CCPTMRS0bits.C2TSEL = 0;        // Utilise TMR2.

    CCP3CONbits.CCP3M = 12;         // Sorties P3A et P3B actives à niveau haut.
    CCP3CONbits.P3M1 = 0;           // Contrôleur de demi pont (P3A et P3B).
    PWM3CONbits.P3DC = TEMPS_MORT;  // Temps mort entre sorties complémentaires.
    CCPTMRS0bits.C3TSEL = 0;        // Utilise TMR2.

    // Active les interruptions en général:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    // Active les interruptions du temporisateur 1:
    PIE1bits.TMR1IE = 1;    // Active les interruptions.
    IPR1bits.TMR1IP = 0;    // Interruptions de basse priorité.
    
    // Active les interruptions du temporisateur 2:
    PIE1bits.TMR2IE = 1;    // Active les interruptions.
    IPR1bits.TMR2IP = 0;    // Interruptions de basse priorité.

    // Configure les ports IO:
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    TRISA = 0xFF;       // Tous les bits du port A comme entrées.
    TRISB = 0b0011110;  // Entrées analogiques du port B.
    TRISC = 0x00;       // Tous les bits du port C comme sorties.

    // Surveille la file d'événements, et les traite au fur
    // et à mesure:
    while(fileDeborde() == 0) {
        ev = defileEvenement();
        if (ev != 0) {
            do {
                MOTEUR_machine(ev);
                PUISSANCE_machine(ev);
                ev = defileMessageInterne();
            } while (ev != 0);
        }
    }

    // La file a débordé:
    CCPR1L = 0;
    CCPR2L = 0;
    CCPR3L = 0;

    // Tout s'arrête:
    while(1);
}

#else

/**
 * Point d'entrée pour les tests unitaires.
 */
void main() {
    unsigned char ft = 0;

    // Initialise la EUSART pour pouvoir écrire dans la console
    // Activez la UART1 dans les propriétés du projet.
    EUSART_Initialize();
    printf("Lancement des tests...\r\n");
    
    // Configure tous les ports comme entrées:
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0x00;
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;

    // Exécution des tests:
    ft += test_tableauDeBord();
    ft += test_file();
    ft += test_puissance();
    ft += test_moteur();

    // Affiche le résultat des tests:
    printf("%u tests en erreur\r\n",ft);
    SLEEP();
}
#endif