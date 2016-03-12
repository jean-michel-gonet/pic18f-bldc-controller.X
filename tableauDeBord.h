#include "domaine.h"

#ifndef __TABLEAU_DE_BORD_H
#define __TABLEAU_DE_BORD_H

typedef union {
    unsigned int valeur;
    struct {
        unsigned char basse;
        unsigned char haute;
    } partie;
} Compteur;

typedef struct {
    Compteur tempsHaut;
    Compteur tempsBas;
} GenerateurPWMServo;

/**
 * Le tableau de bord contient l'état interne du système.
 * Lorsqu'un module change une valeur du tableau de bord, il
 * doit émettre un message interne.
 */
typedef struct {
    /** Dernière vitesse mesurée (CHANGEMENT_VITESSE_MESUREE). */
    MagnitudeEtDirection vitesseMesuree;
    
    /** Tension moyenne d'alimentation du moteur (CHANGEMENT_TENSION_MOYENNE). */
    MagnitudeEtDirection tensionMoyenne;
    
    /** Position du volant (CHANGEMENT_POSITION_ROUES_AVANT). */
    GenerateurPWMServo positionRouesAvant;
    
} TableauDeBord;

/** Le tableau de bord est une variable globale. */
TableauDeBord tableauDeBord = {
    {INDETERMINEE, 0},              // Vitesse mesurée.
    {INDETERMINEE, 0},              // Tension moyenne à appliquer.
    {65535 - 3000, 65535 - 37000}   // Position des roues avant.
};

/**
 * Ajoute un message interne dans la queue.
 * Les messages internes n'ont pas de valeur associée, car ils se réfèrent
 * toujours à des changements du {@link TableauDeBord}.
 * @param evenement Identifiant du message interne.
 */
void enfileMessageInterne(Evenement evenement);

/**
 * Récupère un message interne de la queue.
 * Les messages internes n'ont pas de valeur associée car ils se réfèrent
 * toujours à des changements du {@link TableauDeBord}.
 * @return L'identifiant du message interne.
 */
EvenementEtValeur *defileMessageInterne();

/**
 * Réinitialise la file des messages internes.
 * Utilisée pour les tests unitaires.
 */
void reinitialiseMessagesInternes();

#ifdef TEST
unsigned char test_tableauDeBord();
#endif

#endif