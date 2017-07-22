#include "domaine.h"

#ifndef __EVENEMENTS_H
#define __EVENEMENTS_H

/**
 * Réinitialise la file d'événements.
 */
void initialiseEvenements();

/**
 * Ajoute un événement à la file.
 * @param evenement événement.
 * @param valeur Valeur associée.
 */
void enfileEvenement(enum EVENEMENT evenement, unsigned char valeur);

/**
 * Récupère un événement de la file.
 * @return L'événement.
 */
struct EVENEMENT_ET_VALEUR *defileEvenement();

/**
 * Indique si la file a débordé.
 * @return 0 tant que la file n'a pas débordé.
 */
unsigned char fileDeborde();


#ifdef TEST
/** Tests unitaires pour les événements. */
void test_evenements();
#endif

#endif