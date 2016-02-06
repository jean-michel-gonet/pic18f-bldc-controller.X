#include "domaine.h"

#ifndef __PUISSANCE_H
#define __PUISSANCE_H

/**
 * Varie la puissance selon la vitesse demandée et la vitesse mesurée.
 * @param vitesseMesuree Dernière vitesse mesurée (vitesse réelle, vitesse actuelle).
 * @param vitesseDemandee Vitesse demandée.
 * @return Puissance à appliquer.
 */
MagnitudeEtDirection *calculeTensionMoyenne(MagnitudeEtDirection *vitesseMesuree, 
                                            MagnitudeEtDirection *vitesseDemandee);
#ifdef TEST
/**
 * Tests unitaires pour le calcul de puissance.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance();
#endif

#endif