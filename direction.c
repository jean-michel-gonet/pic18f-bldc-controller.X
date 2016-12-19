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
unsigned test_calculePwmServoRouesAvant() {
    unsigned char testsEnErreur = 0;
    
    calculePwmServoRouesAvant(128);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 3024, "DIR01");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 3024, "DIR01a");

    calculePwmServoRouesAvant(0);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 2000, "DIR11");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 2000, "DIR11a");

    calculePwmServoRouesAvant(255);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 4040, "DIR21");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 4040, "DIR21a");

    return testsEnErreur;
}

unsigned test_directionMachine() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur ev = {LECTURE_RC_GAUCHE_DROITE, 0};

    DIRECTION_machine(&ev);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 2000, "DIRM11");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 2000, "DIRM11a");

    return testsEnErreur;
}



/**
 * Tests unitaires pour le positionnement des roues de direction.
 * @return Nombre de tests en erreur.
 */
unsigned char test_direction() {
    unsigned char testsEnErreur = 0;
    
    testsEnErreur += test_calculePwmServoRouesAvant();
    testsEnErreur += test_directionMachine();
    
    return testsEnErreur;
}
#endif
