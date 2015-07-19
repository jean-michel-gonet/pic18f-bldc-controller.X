#include "puissance.h"
#include "test.h"

#define LIMITE_CROISSANCE_PUISSANCE 8
#define PUISSANCE_MAXIMUM 200
#define INITIALISATION_PID -32767

/**
 * Puissance actuelle.
 */
static unsigned char puissance = 0;

/**
 * Erreur de vitesse précédent.
 */
static signed int e0 = INITIALISATION_PID;

/**
 * Etablit la puissance de départ.
 * Calcule la puissance à appliquer sans tenir compte de la variation de vitesse
 * depuis de dernier calcul. Est utilisée pour effectuer le premier calcul
 * de puissance, lorsque le moteur était en arrêt.
 * @param vitesseMesuree Dernière vitesse mesurée. Normalement 0.
 * @param vitesse Vitesse demandée.
 * @return Puissance à appliquer.
 */
unsigned char calculePuissanceInitiale(unsigned char vitesseMesuree, unsigned char vitesseDemandee) {
    puissance = 0;
    e0 = INITIALISATION_PID;

    return calculePuissance(vitesseMesuree, vitesseDemandee);
}

/**
 * Varie la puissance selon la vitesse demandée et la vitesse mesurée.
 * @param vitesseMesuree Dernière vitesse mesurée (vitesse réelle, vitesse actuelle).
 * @param vitesseDemandee Vitesse demandée.
 * @return Puissance à appliquer.
 */
unsigned char calculePuissance(unsigned char vitesseMesuree, unsigned char vitesseDemandee) {
    signed int e, de;
    signed int dp;

    // Erreur de vitesse:
    e = vitesseDemandee - vitesseMesuree;

    // Variation de l'erreur:
    if (e0 != INITIALISATION_PID) {
        de = e - e0;
    } else {
        de = 0;
    }
    e0 = e;

    // Variation de puissance:
    dp = e / 2  + de / 4;
    if (dp > 0) {
        if (dp > LIMITE_CROISSANCE_PUISSANCE) {
            dp = 8;
        }
        if (puissance < PUISSANCE_MAXIMUM) {
            puissance +=dp;
        }
    } else {
        if (dp < -LIMITE_CROISSANCE_PUISSANCE) {
            dp = -8;
        }
        if (puissance > LIMITE_CROISSANCE_PUISSANCE) {
            puissance += dp;
        }
    }

    // Rend la puissance:
    return puissance;
}

#ifdef TEST
/**
 * Tests unitaires pour le calcul de puissance.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance() {
    unsigned char ft = 0;
    unsigned char n;
    unsigned char puissance;

    int vitesseMesuree;

    // Tests avec réponse sans inertie:
    vitesseMesuree = 0;
    puissance = calculePuissanceInitiale(vitesseMesuree, 100);
    for (n = 0; n < 50; n++) {
        vitesseMesuree = (puissance * 5) / 2;
        puissance = calculePuissance(vitesseMesuree, 100);
    }
    ft += assertMinMaxInt(vitesseMesuree, 95, 105, "PID-ACC52");

    for (n = 0; n < 50; n++) {
        vitesseMesuree = (puissance * 5) / 2;
        puissance = calculePuissance(vitesseMesuree, 25);
    }
    ft += assertMinMaxInt(vitesseMesuree, 20, 30, "PID-DEC52");

    vitesseMesuree = 0;
    puissance = calculePuissanceInitiale(vitesseMesuree, 100);
    for (n = 0; n < 50; n++) {
        vitesseMesuree = (puissance * 2) / 5;
        puissance = calculePuissance(vitesseMesuree, 100);
    }
    ft += assertMinMaxInt(vitesseMesuree, 95, 105, "PID-ACC25");

    for (n = 0; n < 50; n++) {
        vitesseMesuree = (puissance * 2) / 5;
        puissance = calculePuissance(vitesseMesuree, 25);
    }
    ft += assertMinMaxInt(vitesseMesuree, 20, 30, "PID-DEC25");


    // Tests avec réponse avec intertie:
    vitesseMesuree = 0;
    puissance = calculePuissanceInitiale(vitesseMesuree, 100);
    for (n = 0; n < 50; n++) {
        vitesseMesuree = (vitesseMesuree * 3 + (puissance * 5) / 2) / 4;
        puissance = calculePuissance(vitesseMesuree, 100);
    }
    ft += assertMinMaxInt(vitesseMesuree, 95, 105, "PID-ACCI");

    for (n = 0; n < 50; n++) {
        vitesseMesuree = (vitesseMesuree * 3 + (puissance * 5) / 2) / 4;
        puissance = calculePuissance(vitesseMesuree, 25);
    }
    ft += assertMinMaxInt(vitesseMesuree, 20, 30, "PID-DECI");

    return ft;
}
#endif