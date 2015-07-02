#ifndef __PUISSANCE_H
#define __PUISSANCE_H

#define VITESSE_MAX 29

/**
 * Etablit la puissance de départ.
 * Est utilisée pour initialiser la puissance à une valeur possible,
 * en particulier suite à un démarrage ou à une situation de blocage.
 * @param p La puissance de départ.
 */
void etablitPuissance(unsigned char p);

/**
 * Calcule la puissance selon la vitesse demandée et la durée de la dernière phase.
 * @param dureeDePhase Durée de phase actuelle.
 * @param vitesse Vitesse demandée.
 * @return Puissance à appliquer.
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