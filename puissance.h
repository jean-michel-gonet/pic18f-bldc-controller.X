#ifndef __PUISSANCE_H
#define __PUISSANCE_H

/**
 * Etablit la puissance de départ.
 * Calcule la puissance à appliquer sans tenir compte de la variation de vitesse
 * depuis de dernier calcul. Est utilisée pour effectuer le premier calcul
 * de puissance, lorsque le moteur était en arrêt.
 * @param vitesseMesuree Dernière vitesse mesurée. Normalement 0.
 * @param vitesse Vitesse demandée.
 * @return Puissance à appliquer.
 */
unsigned char calculeTensionMoyenneInitiale(unsigned char vitesseMesuree, unsigned char vitesseDemandee);

/**
 * Varie la puissance selon la vitesse demandée et la vitesse mesurée.
 * @param vitesseMesuree Dernière vitesse mesurée (vitesse réelle, vitesse actuelle).
 * @param vitesseDemandee Vitesse demandée.
 * @return Puissance à appliquer.
 */
unsigned char calculeTensionMoyenne(unsigned char vitesseMesuree, unsigned char vitesseDemandee);

#ifdef TEST
/**
 * Tests unitaires pour le calcul de puissance.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance();
#endif

#endif