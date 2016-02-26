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
#include "direction.h"

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

typedef enum {
    TEMPS_HAUT,
    TEMPS_BAS
} EtatGenerateurPWMServo;

/**
 * Routine de traitement des interruptions de haute priorité.
 * Utilisée pour produire le signal PWM destiné à diriger les roues avant
 * de la voiture.
 */
void interrupt interruptionsHautePriorite() {
    static EtatGenerateurPWMServo etat = TEMPS_BAS;
    
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        switch (etat) {
            case TEMPS_HAUT:
                etat = TEMPS_BAS;
                TMR0H = tableauDeBord.positionRouesAvant.tempsBas.partie.haute;
                TMR0L = tableauDeBord.positionRouesAvant.tempsBas.partie.basse;
                PORTAbits.RA6 = 0;
                break;

            case TEMPS_BAS:
                etat = TEMPS_HAUT;
                TMR0H = tableauDeBord.positionRouesAvant.tempsHaut.partie.haute;
                TMR0L = tableauDeBord.positionRouesAvant.tempsHaut.partie.basse;
                PORTAbits.RA6 = 1;
                break;
        }
    }
}

/**
 * Routine de traitement d'interruptions de basse priorité.
 * Pilotage du moteur sur la base des détecteurs Hall.
 */
void low_priority interrupt interruptionsBassePriorite() {
    unsigned char hall;
    static unsigned char hall0 = 0;
    static int tempsMesureVitesse = TEMPS_BASE_DE_TEMPS;
    static unsigned int instantCapture4, instantCapture5;

    // Traitement des conversions AD:
    if (PIR5bits.TMR4IF) {
        PIR5bits.TMR4IF = 0;

        if (!ADCON0bits.GODONE) {
            enfileEvenement(LECTURE_POTENTIOMETRE, ADRESH);
            ADCON0bits.GODONE = 1;
        }
    }

    // Capture de l'entrée CCP4:
    if (PIR4bits.CCP4IF) {
        PIR4bits.CCP4IF = 0;
        if (PORTBbits.RB0) {
            CCP4CONbits.CCP4M = 4;          // Capture du flanc descendant.
            instantCapture4 = CCPR4;
        } else {
            CCP4CONbits.CCP4M = 5;          // Capture du flanc montant.

            instantCapture4 = CCPR4 - instantCapture4;
            instantCapture4 -= 2000;
            instantCapture4 >>= 3;
            enfileEvenement(LECTURE_RC_GAUCHE_DROITE, (unsigned char) instantCapture4);
        }
    }

    // Capture de l'entrée CCP5:
    if (PIR4bits.CCP5IF) {
        PIR4bits.CCP5IF = 0;
        if (PORTAbits.RA4) {
            CCP5CONbits.CCP5M = 4;          // Capture du flanc descendant.
            instantCapture5 = CCPR5;
        } else {
            CCP5CONbits.CCP5M = 5;          // Capture du flanc montant.

            instantCapture5 = CCPR5 - instantCapture5;
            if (instantCapture5 > 4000) {
                instantCapture5 = 4000;
            }
            if (instantCapture5 < 2000) {
                instantCapture5 = 2000;
            }
            instantCapture5 -= 2000;
            instantCapture5 >>= 3;
            enfileEvenement(LECTURE_RC_AVANT_ARRIERE, (unsigned char) instantCapture5);
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

    // Temporisateur 0: PWM pour le servo de direction.
    T0CONbits.T08BIT = 0;       // Compteur de 16 bits.
    T0CONbits.T0CS = 0;         // Source: FOSC / 4
    T0CONbits.PSA = 0;          // Active le diviseur de fréquence.
    T0CONbits.T0PS = 2;         // Diviseur de fréquence TPS = 8
    T0CONbits.TMR0ON = 1;       // Active le temporisateur
    
    INTCONbits.TMR0IE = 1;      // Active les interruptions.
    INTCON2bits.TMR0IP = 1;     // Interruptions de haute priorité.

    // Temporisateur 1: Capture CCP4 et CCP5 (2ms ==> 4000)
    T1CONbits.TMR1CS = 0;       // Source: FOSC / 4
    T1CONbits.T1CKPS = 3;       // Diviseur de fréquence TPS = 8
    T1CONbits.T1RD16 = 1;       // Temporisateur de 16 bits.
    T1CONbits.TMR1ON = 1;       // Active le temporisateur 1

    // Temporisateur 2: PWM pour le moteur.
    T2CONbits.T2CKPS = 1;       // Pas de division de fréquence.
    T2CONbits.T2OUTPS = 0;      // Pas de division de fréquence.
    T2CONbits.TMR2ON = 1;       // Active le temporisateur.
    PR2 = 255;                  // Période max: 64MHz / (4 * 255) = 62kHz.
    
    PIE1bits.TMR2IE = 1;        // Active les interruptions.
    IPR1bits.TMR2IP = 0;        // Interruptions de basse priorité.

    // Temporisateur 4: Cadence des conversions A/D:
    T4CONbits.T4CKPS = 1;       // Pas de division de fréquence.
    T4CONbits.T4OUTPS = 0;      // Pas de division de fréquence.
    T4CONbits.TMR4ON = 1;       // Active le temporisateur

    PIE5bits.TMR4IE = 1;        // Active les interruptions.
    IPR5bits.TMR4IP = 0;        // Interruptions de basse priorité.
    
    // Active les CCP 1 à 3 en mode PWM, tous sur le TMR2:
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

    // Active les CCP4 et CCP5 en mode capture, tous sur le TMR 1:
    CCP4CONbits.CCP4M = 5;          // Capture du flanc montant.
    CCPTMRS1bits.C4TSEL = 0;        // Utilise TMR1
    PIE4bits.CCP4IE = 1;            // Active les interruptions.
    IPR4bits.CCP4IP = 0;            // Basse priorité.
    
    CCP5CONbits.CCP5M = 5;          // Capture du flanc montant.
    CCPTMRS1bits.C5TSEL = 0;        // Utilise TMR1
    PIE4bits.CCP5IE = 1;            // Active les interruptions.
    IPR4bits.CCP5IP = 0;            // Basse priorité.
    
    // Active les interruptions en général:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    // Configure les ports IO:
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    TRISA = 0b10111111;  // RA6 est une sortie.
    TRISB = 0b00011111;  // Entrées analogiques du port B.
    TRISC = 0x00;        // Tous les bits du port C comme sorties.

    // Surveille la file d'événements, et les traite au fur
    // et à mesure:
    while(fileDeborde() == 0) {
        ev = defileEvenement();
        if (ev != 0) {
            do {
                MOTEUR_machine(ev);
                PUISSANCE_machine(ev);
                DIRECTION_machine(ev);
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
    ft += test_direction();

    // Affiche le résultat des tests:
    printf("%u tests en erreur\r\n",ft);
    SLEEP();
}
#endif