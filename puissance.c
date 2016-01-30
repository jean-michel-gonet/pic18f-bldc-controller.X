#include "puissance.h"
#include "test.h"

#define LIMITE_CROISSANCE_TENSION 8
#define TENSION_MAXIMUM 200
#define INITIALISATION_PID -32767

/**
 * Tension actuelle.
 */
static unsigned char tension = 0;

/**
 * Erreur de vitesse précédent.
 */
static signed int e0 = INITIALISATION_PID;

/**
 * Etablit la tension moyenne de départ.
 * Calcule la tension moyenne à appliquer sans tenir compte de la variation de vitesse
 * depuis de dernier calcul. Est utilisée pour effectuer le premier calcul
 * de tension, lorsque le moteur était en arrêt.
 * @param vitesseMesuree Dernière vitesse mesurée. Normalement 0.
 * @param vitesse Vitesse demandée.
 * @return Tension moyenne à appliquer, entre 0 et 255 (correspond au cycle
 * de travail d'un PWM).
 */
unsigned char calculeTensionMoyenneInitiale(unsigned char vitesseMesuree, unsigned char vitesseDemandee) {
    tension = 0;
    e0 = INITIALISATION_PID;

    return calculeTensionMoyenne(vitesseMesuree, vitesseDemandee);
}

/**
 * Varie la tension moyenne à appliquer selon la vitesse demandée et la vitesse mesurée.
 * @param vitesseMesuree Dernière vitesse mesurée (vitesse réelle, vitesse actuelle).
 * @param vitesseDemandee Vitesse demandée.
 * @return Tension moyenne à appliquer sur le moteur, entre 0 et 255 (correspond
 * au cycle de travail d'un PWM).
 */
unsigned char calculeTensionMoyenne(unsigned char vitesseMesuree, unsigned char vitesseDemandee) {
    return vitesseDemandee;
}

#ifdef TEST
/**
 * Tests unitaires pour le calcul de tension.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance() {
    unsigned char ft = 0;
    unsigned char n;
    unsigned char tension;

    int vitesseMesuree;

    // Tests avec réponse sans inertie:
    vitesseMesuree = 0;
    tension = calculeTensionMoyenneInitiale(vitesseMesuree, 100);
    for (n = 0; n < 50; n++) {
        vitesseMesuree = (tension * 5) / 2;
        tension = calculeTensionMoyenne(vitesseMesuree, 100);
    }
    ft += assertMinMaxInt(vitesseMesuree, 95, 105, "PID-ACC52");

    for (n = 0; n < 50; n++) {
        vitesseMesuree = (tension * 5) / 2;
        tension = calculeTensionMoyenne(vitesseMesuree, 25);
    }
    ft += assertMinMaxInt(vitesseMesuree, 20, 30, "PID-DEC52");

    vitesseMesuree = 0;
    tension = calculeTensionMoyenneInitiale(vitesseMesuree, 100);
    for (n = 0; n < 50; n++) {
        vitesseMesuree = (tension * 2) / 5;
        tension = calculeTensionMoyenne(vitesseMesuree, 100);
    }
    ft += assertMinMaxInt(vitesseMesuree, 95, 105, "PID-ACC25");

    for (n = 0; n < 50; n++) {
        vitesseMesuree = (tension * 2) / 5;
        tension = calculeTensionMoyenne(vitesseMesuree, 25);
    }
    ft += assertMinMaxInt(vitesseMesuree, 20, 30, "PID-DEC25");


    // Tests avec réponse avec intertie:
    vitesseMesuree = 0;
    tension = calculeTensionMoyenneInitiale(vitesseMesuree, 100);
    for (n = 0; n < 50; n++) {
        vitesseMesuree = (vitesseMesuree * 3 + (tension * 5) / 2) / 4;
        tension = calculeTensionMoyenne(vitesseMesuree, 100);
    }
    ft += assertMinMaxInt(vitesseMesuree, 95, 105, "PID-ACCI");

    for (n = 0; n < 50; n++) {
        vitesseMesuree = (vitesseMesuree * 3 + (tension * 5) / 2) / 4;
        tension = calculeTensionMoyenne(vitesseMesuree, 25);
    }
    ft += assertMinMaxInt(vitesseMesuree, 20, 30, "PID-DECI");

    return ft;
}
#endif