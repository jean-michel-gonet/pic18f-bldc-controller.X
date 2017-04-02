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
unsigned char detecte_une_pulsation_sur_un_canal() {
    unsigned char ft = 0;

    initialiseCapture();

    captureFlancMontant(0, 10000);
    ft += assertEqualsChar(captureFlancDescendant(0, 11000),   0, "CP1C01");

    captureFlancMontant(0, 10000);
    ft += assertEqualsChar(captureFlancDescendant(0, 12000),   0, "CP1C02");

    captureFlancMontant(0, 10000);
    ft += assertEqualsChar(captureFlancDescendant(0, 12500),  62, "CP1C03");

    captureFlancMontant(0, 10000);
    ft += assertEqualsChar(captureFlancDescendant(0, 13000), 125, "CP1C04");
    
    captureFlancMontant(0, 10000);
    ft += assertEqualsChar(captureFlancDescendant(0, 13500), 187, "CP1C05");
    
    captureFlancMontant(0, 10000);
    ft += assertEqualsChar(captureFlancDescendant(0, 14000), 250, "CP1C06");
    
    captureFlancMontant(0, 10000);
    ft += assertEqualsChar(captureFlancDescendant(0, 15000), 250, "CP1C07");

    return ft;
}

unsigned char detecte_des_pulsations_sur_deux_canaux() {
    unsigned char ft = 0;

    initialiseCapture();

    captureFlancMontant(0, 10000);
    captureFlancMontant(1, 11000);
    captureFlancMontant(2, 12000);
    captureFlancMontant(3, 13000);
    captureFlancMontant(4, 14000);

    ft += assertEqualsChar(captureFlancDescendant(0, 13000),   125, "CP2C00");
    ft += assertEqualsChar(captureFlancDescendant(1, 14000),   125, "CP2C01");
    ft += assertEqualsChar(captureFlancDescendant(2, 15000),   125, "CP2C02");
    ft += assertEqualsChar(captureFlancDescendant(3, 16000),   125, "CP2C03");
    ft += assertEqualsChar(captureFlancDescendant(4, 17000),   125, "CP2C04");

    return ft;
}

unsigned char detecte_une_pulsation_avec_debordement() {
    unsigned char ft = 0;

    initialiseCapture();

    captureFlancMontant(0, 65500);

    ft += assertEqualsChar(captureFlancDescendant(0, 2964),   125, "CPDE00");

    return ft;
    
}

unsigned char test_capture() {
    unsigned char ft = 0;
    
    ft += detecte_une_pulsation_sur_un_canal();
    ft += detecte_des_pulsations_sur_deux_canaux();
    ft += detecte_une_pulsation_avec_debordement();
    
    return ft;
}
#endif