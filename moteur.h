#ifndef __MOTEUR_H
#define __MOTEUR_H

#include "domaine.h"

#define X 255

/**
 * Contient les valeurs des rapports cycliques des trois
 * modulations PWM.
 */
struct CCP {
    /** Rapport cyclique pour le connecteur de la bobine A. */
    unsigned char ccpa;
    /** Rapport cyclique pour le connecteur de la bobine B. */
    unsigned char ccpb;
    /** Rapport cyclique pour le connecteur de la bobine C. */
    unsigned char ccpc;
};

/**
 * Rend les valeurs PWM para rapport à la phase spécifiée, à la tension moyenne
 * et à la direction de rotation.
 * @param tensionMoyenne Tension moyenne à utiliser. Il est conseillé de ne pas utiliser
 * une valeur trop forte ici, pour ne pas brûler le circuit.
 * @param phase Phase actuelle, entre 1 et 6.
 * @param direction AVANT ou ARRIERE.
 * @param ccp Structure pour les valeurs PWM.
 */
void calculeAmplitudes(unsigned char tensionMoyenne,
                       enum DIRECTION direction,
                       unsigned char phase,
                       struct CCP *ccp);

/**
 * Determine la phase en cours d'aprés les senseurs hall.
 * @param hall La valeur des senseurs hall: 0b*****zyx
 * @return Le numéro de phase, entre 1 et 6.
 */
unsigned char phaseSelonHall(unsigned char hall);

/**
 * Calcule la phase en cours à partir de la lecture des senseurs hall.
 * Effectue également un contrôle de la lecture, pour vérifier si elle est
 * possible. Ceci sert à éviter de compter des rebondissements ou du bruit
 * qui affecte la lecture des senseurs.
 * @param hall La valeur des senseurs hall: 0b*****yzx
 * @param direction Direction actuelle.
 * @return La phase (de 1 à 6) ou un code d'erreur.
 */
unsigned char phaseSelonHallEtDirection(unsigned char hall, enum DIRECTION direction);

#ifdef TEST
/**
 * Point d'entrée pour les tests du moteur.
 * @return Nombre de tests en erreur.
 */
unsigned char test_moteur();
#endif

#endif