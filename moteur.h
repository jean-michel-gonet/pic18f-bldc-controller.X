#include "domaine.h"
#include "evenements.h"

#ifndef __MOTEUR_H
#define __MOTEUR_H

/**
 * Machine à états pour contrôler le moteur.
 * @param ev Événement à traiter.
 */
void MOTEUR_machine(EvenementEtValeur *ev);

#ifdef TEST
/** Point d'entrée pour les tests du moteur. */
void test_moteur();
#endif

#endif