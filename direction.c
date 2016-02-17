#include "test.h"
#include "tableauDeBord.h"
#include "direction.h"

void calculePwmServoRouesAvant(unsigned char position) {
    unsigned int x = position;
    x <<= 3;
    x += 2000;
    tableauDeBord.positionRouesAvant.tempsBas.valeur = 25535 + x;
    tableauDeBord.positionRouesAvant.tempsHaut.valeur = 65535 - x;
}

/**
 * Machine à états pour réguler la position des roues avant (de direction).
 * @param ev Événement à traiter.
 */
void DIRECTION_machine(EvenementEtValeur *ev) {
    unsigned int x;
    if (ev->evenement == LECTURE_POTENTIOMETRE) {
        calculePwmServoRouesAvant(ev->valeur);
    }
}

#ifdef TEST
unsigned test_calculePwmServoRouesAvant() {
    unsigned char testsEnErreur = 0;
    
    calculePwmServoRouesAvant(128);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 3024, "DIR01");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 40000 - 3024, "DIR01");

    calculePwmServoRouesAvant(0);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 2000, "DIR11");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 40000 - 2000, "DIR11");

    calculePwmServoRouesAvant(255);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 4040, "DIR21");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 40000 - 4040, "DIR21");

    calculePwmServoRouesAvant(160);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 2000 + 160 * 8, "DIR31");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 38000 - 160 * 8, "DIR31");

    calculePwmServoRouesAvant(100);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 2000 + 100 * 8, "DIR41");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 38000 - 100 * 8, "DIR41");

    return testsEnErreur;
}

unsigned test_directionMachine() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur ev = {LECTURE_POTENTIOMETRE, 0};

    DIRECTION_machine(&ev);
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsBas.valeur, 2000, "DIRM11");
    assertEqualsInt(tableauDeBord.positionRouesAvant.tempsHaut.valeur, 40000 - 2000, "DIRM11");

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
