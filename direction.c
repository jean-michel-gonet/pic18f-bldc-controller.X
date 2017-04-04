#include "test.h"
#include "tableauDeBord.h"
#include "direction.h"

void calculePwmServoRouesAvant(unsigned char position) {
    unsigned int x = position;
    x <<= 3;
    x += 2000;
    tableauDeBord.positionRouesAvant.tempsBas.valeur = 25535 + x;
    tableauDeBord.positionRouesAvant.tempsHaut.valeur = (unsigned int) 65535 - x;
}

/**
 * Machine à états pour réguler la position des roues avant (de direction).
 * @param ev Événement à traiter.
 */
void DIRECTION_machine(EvenementEtValeur *ev) {
    unsigned int x;
    if (ev->evenement == LECTURE_RC_GAUCHE_DROITE) {
        calculePwmServoRouesAvant(ev->valeur);
    }
}

#ifdef TEST
unsigned calcule_pwm_servo_roues_avant() {
    unsigned char testsEnErreur = 0;
    
    calculePwmServoRouesAvant(128);
    verifieEgalite("DIR01", tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 3024);
    verifieEgalite("DIR01a", tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 3024);

    calculePwmServoRouesAvant(0);
    verifieEgalite("DIR11", tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 2000);
    verifieEgalite("DIR11a", tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 2000);

    calculePwmServoRouesAvant(255);
    verifieEgalite("DIR21", tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 4040);
    verifieEgalite("DIR21a", tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 4040);

    return testsEnErreur;
}

unsigned test_directionMachine() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur ev = {LECTURE_RC_GAUCHE_DROITE, 0};

    DIRECTION_machine(&ev);
    verifieEgalite("DIRM11", tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 2000);
    verifieEgalite("DIRM11a", tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 2000);

    return testsEnErreur;
}



/**
 * Tests unitaires pour le positionnement des roues de direction.
 */
void test_direction() {
    calcule_pwm_servo_roues_avant();
    test_directionMachine();
}
#endif
