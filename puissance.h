#include "domaine.h"

#ifndef __PUISSANCE_H
#define __PUISSANCE_H

/**
 * Machine à états pour réguler la puissance (tension moyenne) appliquée
 * au moteur.
 * @param ev Événement à traiter.
 */
void PUISSANCE_machine(EvenementEtValeur *ev);

#ifdef TEST
/**
 * Tests unitaires pour le calcul de puissance.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance();
#endif

#endif