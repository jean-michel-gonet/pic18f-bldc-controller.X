#ifndef __PUISSANCE_H
#define __PUISSANCE_H

#define VITESSE_MAX 200

/**
 * Etablit la puissance de d�part.
 * Est utilis�e pour initialiser la puissance � une valeur possible,
 * en particulier suite � un d�marrage ou � une situation de blocage.
 * @param p La puissance de d�part.
 */
void etablitPuissance(unsigned char p);

/**
 * Calcule la puissance selon la vitesse demand�e et la dur�e de la derni�re phase.
 * @param dureeDePhase Dur�e de phase actuelle.
 * @param vitesse Vitesse demand�e.
 * @return Puissance � appliquer.
 */
unsigned char calculePuissance(int dureeDePhaseActuelle, unsigned char vitesseDemandee);

#ifdef TEST
/**
 * Tests unitaires pour le calcul de puissance.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance();
#endif

#endif