#ifndef DIRECTION_H
#define	DIRECTION_H

/**
 * Machine à états pour réguler la position des roues avant (de direction).
 * @param ev Événement à traiter.
 */
void DIRECTION_machine(EvenementEtValeur *ev);

void receptionBus(unsigned char address, unsigned char valeur);
void receptionTelecommandeAvantArriere(unsigned char valeur);
void receptionTelecommandeGaucheDroite(unsigned char valeur);


#ifdef TEST
void test_direction();
#endif

#endif

