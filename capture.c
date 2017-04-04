#include "capture.h"
#include "test.h"

/** Instants capturés des flancs montants. */
unsigned int instantFlancMontant[] = {0, 0, 0, 0, 0};

/**
 * Initialise la capture.
 */
void initialiseCapture() {
    instantFlancMontant[0] = 0;
    instantFlancMontant[1] = 0;
    instantFlancMontant[2] = 0;
    instantFlancMontant[3] = 0;
    instantFlancMontant[4] = 0;
}

/**
 * Signale la capture d'un flanc montant.
 * @param canal Canal où a été capturé le flanc.
 * @param instant Instant au quel a été capturé le flanc.
 */
void captureFlancMontant(unsigned char canal, unsigned int instant) {
    instantFlancMontant[canal] = instant;
}

/**
 * Signale la capture d'un flanc descendant.
 * @param canal Canal où a été capturé le flanc.
 * @param instant Instant au quel a été capturé le flanc.
 * @return Une valeur entre 0 et 255 proportionnelle à la durée de
 * la partie haute de la pulsation.
 */
unsigned char captureFlancDescendant(unsigned char canal, unsigned int instant) {
    instant -= instantFlancMontant[canal];
    if (instant > 4000) {
        instant = 4000;
    }
    if (instant < 2000) {
        instant = 2000;
    }
    instant -= 2000;
    instant >>= 3;
    
    return instant;
}

#ifdef TEST
void detecte_une_pulsation_sur_un_canal() {
    initialiseCapture();

    captureFlancMontant(0, 10000);
    verifieEgalite("CP1C01", captureFlancDescendant(0, 11000),   0);

    captureFlancMontant(0, 10000);
    verifieEgalite("CP1C02", captureFlancDescendant(0, 12000),   0);

    captureFlancMontant(0, 10000);
    verifieEgalite("CP1C03", captureFlancDescendant(0, 12500),  62);

    captureFlancMontant(0, 10000);
    verifieEgalite("CP1C04", captureFlancDescendant(0, 13000), 125);
    
    captureFlancMontant(0, 10000);
    verifieEgalite("CP1C05", captureFlancDescendant(0, 13500), 187);
    
    captureFlancMontant(0, 10000);
    verifieEgalite("CP1C06", captureFlancDescendant(0, 14000), 250);
    
    captureFlancMontant(0, 10000);
    verifieEgalite("CP1C07", captureFlancDescendant(0, 15000), 250);
}

void detecte_des_pulsations_sur_deux_canaux() {
    initialiseCapture();

    captureFlancMontant(0, 10000);
    captureFlancMontant(1, 11000);
    captureFlancMontant(2, 12000);
    captureFlancMontant(3, 13000);
    captureFlancMontant(4, 14000);

    verifieEgalite("CP2C00", captureFlancDescendant(0, 13000),   125);
    verifieEgalite("CP2C01", captureFlancDescendant(1, 14000),   125);
    verifieEgalite("CP2C02", captureFlancDescendant(2, 15000),   125);
    verifieEgalite("CP2C03", captureFlancDescendant(3, 16000),   125);
    verifieEgalite("CP2C04", captureFlancDescendant(4, 17000),   125);
}

void detecte_une_pulsation_avec_debordement() {
    initialiseCapture();

    captureFlancMontant(0, 65500);

    verifieEgalite("CPDE00", captureFlancDescendant(0, 2964),   125);
}

void test_capture() {
    detecte_une_pulsation_sur_un_canal();
    detecte_des_pulsations_sur_deux_canaux();
    detecte_une_pulsation_avec_debordement();
}
#endif