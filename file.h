#ifndef __FILE_H
#define __FILE_H

#include "domaine.h"

/**
 * Groupe un événement et sa valeur associée.
 */
struct EVENEMENT_ET_VALEUR {
    /** L'événement. */
    enum EVENEMENT evenement;
    /** Sa valeur associée. */
    unsigned char valeur;
};

/**
 * Si différent de zéro, alors la file a débordé.
 */
unsigned char file_alerte = 0;

/**
 * Ajoute un événement à la file.
 * @param evenement Événement.
 * @param valeur Valeur associée.
 */
void enfileEvenement(enum EVENEMENT evenement, unsigned char valeur);

/**
 * Récupère un événement de la file.
 * @return L'événement.
 */
struct EVENEMENT_ET_VALEUR *defileEvenement();

#ifdef TEST
/**
 * Tests unitaires pour la file.
 * @return Nombre de tests en erreur.
 */
unsigned char test_file();
#endif

#endif