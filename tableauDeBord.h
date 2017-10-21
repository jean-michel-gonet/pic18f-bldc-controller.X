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
    /** Dernière vitesse mesurée. */
    MagnitudeEtDirection vitesseMesuree;
            
    /** Vitesse demandée. */
    MagnitudeEtDirection vitesseDemandee;

    /** Dernier déplacement mesuré. */
    MagnitudeEtDirection deplacementMesure;
    
    /** Déplacement demandé. */
    MagnitudeEtDirection deplacementDemande;

    /** Tension moyenne d'alimentation du moteur. */
    MagnitudeEtDirection tensionMoyenne;

    /** Position du volant (CHANGEMENT_POSITION_ROUES_AVANT). */
    GenerateurPWMServo positionRouesAvant;
    
    /** Temps écoulé depuis le dernier changement de phase */
    unsigned char tempsDeDeplacement;
    
} TableauDeBord;

/** Le tableau de bord est une variable globale. */
TableauDeBord tableauDeBord = {
    {AVANT, 0},              // Vitesse mesurée.
    {AVANT, 0},              // Vitesse demandée.
    {AVANT, 0},              // Déplacement mesuré.
    {AVANT, 0},              // Déplacement demandé.
    {AVANT, 0},              // Tension moyenne à appliquer.
    {65535 - 3000, 65535 - 37000},   // Position des roues avant.
    0                        // Temps depuis le changement de phase.
};

void enfileMessageInterne(Evenement evenement, unsigned char valeur);
EvenementEtValeur *defileMessageInterne();
void initialiseMessagesInternes();
void initialiseTableauDeBord();


#ifdef TEST
void test_tableauDeBord();
#endif

#endif