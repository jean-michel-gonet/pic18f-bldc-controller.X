#ifndef DIRECTION_H
#define	DIRECTION_H

/**
 * Machine à états pour réguler la position des roues avant (de direction).
 * @param ev Événement à traiter.
 */
void DIRECTION_machine(EvenementEtValeur *ev);

#ifdef TEST
unsigned char test_direction();
#endif

#endif

