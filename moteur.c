#include "domaine.h"
#include "test.h"
#include "moteur.h"

#define OO X
//                                Direction:              AVANT      //        ARRIERE
//                                    Phase:        1  2  3  4  5  6 //     1  2  3  4  5  6
const unsigned char const pwmAParPhase[2][7] = { {OO, X, 1, 1, X, 0, 0}, {OO, X, 0, 0, X, 1, 1} };
const unsigned char const pwmBParPhase[2][7] = { {OO, 0, 0, X, 1, 1, X}, {OO, 1, 1, X, 0, 0, X} };
const unsigned char const pwmCParPhase[2][7] = { {OO, 1, X, 0, 0, X, 1}, {OO, 0, X, 1, 1, X, 0} };

/**
 * Rend les valeurs PWM para rapport à la phase spécifiée, à la puissance
 * et à la direction de rotation.
 * @param puissance Puissance à utiliser. Il est conseillé de ne pas utiliser
 * une valeur trop forte ici, pour ne pas brûler le circuit.
 * @param phase Phase actuelle, entre 1 et 6.
 * @param direction AVANT ou ARRIERE.
 * @param ccp Structure pour les valeurs PWM.
 */
void calculeAmplitudes(unsigned char puissance,
                       enum DIRECTION direction,
                       unsigned char phase,
                       struct CCP *ccp) {
    unsigned char pwm;

    if ( (phase == 0) || (phase > 6) ) {
        ccp->ccpa = 0;
        ccp->ccpb = 0;
        ccp->ccpc = 0;
    } else {
        pwm = pwmAParPhase[direction][phase];
        ccp->ccpa = (pwm == 1 ? puissance : pwm);

        pwm = pwmBParPhase[direction][phase];
        ccp->ccpb = (pwm == 1 ? puissance : pwm);
        
        pwm = pwmCParPhase[direction][phase];
        ccp->ccpc = (pwm == 1 ? puissance : pwm);
    }
}

/*
 * Relation entre valeurs des senseurs Hall et numéro de phase
 */
const unsigned char const phaseParHall[] = {
    0,
    1, 3, 2, 5, 6, 4,
    0
};

/**
 * Determine la phase en cours d'aprés les senseurs hall.
 * @param hall La valeur des senseurs hall: 0b*****zyx
 * @return Le numéro de phase, entre 1 et 6.
 */
unsigned char phaseSelonHall(unsigned char hall) {
    // Vérifie que la nouvelle valeur est possible:
    hall = hall & 7;
    if ((hall == 0) || (hall == 7)) {
        return ERROR;
    }

    // Rend le numéro de phase correspondant aux senseurs hall:
    return phaseParHall[hall];
}

/**
 * Calcule la phase en cours à partir de la lecture des senseurs hall.
 * Effectue également un contrôle de la lecture, pour v�rifier si elle est
 * possible. Ceci sert à éviter de compter des rebondissements ou du bruit
 * qui affecte la lecture des senseurs.
 * @param hall La valeur des senseurs hall: 0b*****yzx
 * @param direction Direction actuelle.
 * @return La phase (de 1 à 6) ou un code d'erreur.
 */
unsigned char phaseSelonHallEtDirection(unsigned char hall, enum DIRECTION direction) {

    static unsigned char phase0 = ERROR;
    unsigned char phase;
    signed char step;

    // Obtient la phase selon les senseurs hall:
    phase = phaseSelonHall(hall);

    // Vérifie que la nouvelle valeur est possible, compte tenu de la
    // valeur précédente, et de la direction:
    // (les senseurs hall peuvent avoir des rebondissements)
    if (phase0 != ERROR) {
        step = phase0 - phase;
        switch(step) {
            case -1:
            case 5:
                if (direction == ARRIERE) {
                    return ERROR;
                }
                break;

            case 1:
            case -5:
                if (direction == AVANT) {
                    return ERROR;
                }
                break;

            case 0:
            default:
                return ERROR;
        }
    }

    // Si on arrive ici, la lecture des Hall est consid�r�e possible,
    // et on peut rendre la phase correspondante:
    phase0 = phase;
    return phase;
}

#ifdef TEST

unsigned char test_phaseSelonHall() {
    unsigned char ft = 0;

    ft += assertEqualsChar(ERROR, phaseSelonHall(0), "PSH-00");
    ft += assertEqualsChar(1, phaseSelonHall(0b001), "PSH-01");
    ft += assertEqualsChar(2, phaseSelonHall(0b011), "PSH-02");
    ft += assertEqualsChar(3, phaseSelonHall(0b010), "PSH-03");
    ft += assertEqualsChar(4, phaseSelonHall(0b110), "PSH-04");
    ft += assertEqualsChar(5, phaseSelonHall(0b100), "PSH-05");
    ft += assertEqualsChar(6, phaseSelonHall(0b101), "PSH-06");
    ft += assertEqualsChar(ERROR, phaseSelonHall(7), "PSH-07");

    return ft;
}

unsigned char test_calculeAmplitudesArret() {
    unsigned char ft = 0;
    unsigned char puissance = 15;
    struct CCP ccp;

    // Phase inconnue:
    calculeAmplitudes(puissance, AVANT, 0, &ccp);
    ft += assertEqualsChar(ccp.ccpa, 0, "CAA-0A");
    ft += assertEqualsChar(ccp.ccpb, 0, "CAA-0B");
    ft += assertEqualsChar(ccp.ccpc, 0, "CAA-0C");

    calculeAmplitudes(puissance, AVANT, 7, &ccp);
    ft += assertEqualsChar(ccp.ccpa, 0, "CAA-0A");
    ft += assertEqualsChar(ccp.ccpb, 0, "CAA-0B");
    ft += assertEqualsChar(ccp.ccpc, 0, "CAA-0C");

    // Marche avant:
    calculeAmplitudes(puissance, AVANT, 1, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAV-1A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAV-1B");
    ft += assertEqualsChar(ccp.ccpc, puissance, "CAV-1C");

    calculeAmplitudes(puissance, AVANT, 2, &ccp);
    ft += assertEqualsChar(ccp.ccpa, puissance, "CAV-2A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAV-2B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAV-2C");

    calculeAmplitudes(puissance, AVANT, 3, &ccp);
    ft += assertEqualsChar(ccp.ccpa, puissance, "CAV-3A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAV-3B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAV-3C");

    calculeAmplitudes(puissance, AVANT, 4, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAV-4A");
    ft += assertEqualsChar(ccp.ccpb, puissance, "CAV-4B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAV-4C");

    calculeAmplitudes(puissance, AVANT, 5, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAV-5A");
    ft += assertEqualsChar(ccp.ccpb, puissance, "CAV-5B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAV-5C");

    calculeAmplitudes(puissance, AVANT, 6, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAV-6A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAV-6B");
    ft += assertEqualsChar(ccp.ccpc, puissance, "CAV-6C");


    // Marche arrière:
    calculeAmplitudes(puissance, ARRIERE, 1, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAR-1A");
    ft += assertEqualsChar(ccp.ccpb, puissance, "CAR-1B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAR-1C");

    calculeAmplitudes(puissance, ARRIERE, 2, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAR-2A");
    ft += assertEqualsChar(ccp.ccpb, puissance, "CAR-2B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAR-2C");

    calculeAmplitudes(puissance, ARRIERE, 3, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAR-3A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAR-3B");
    ft += assertEqualsChar(ccp.ccpc, puissance, "CAR-3C");

    calculeAmplitudes(puissance, ARRIERE, 4, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAR-4A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAR-4B");
    ft += assertEqualsChar(ccp.ccpc, puissance, "CAR-4C");

    calculeAmplitudes(puissance, ARRIERE, 5, &ccp);
    ft += assertEqualsChar(ccp.ccpa, puissance, "CAR-5A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAR-5B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAR-5C");

    calculeAmplitudes(puissance, ARRIERE, 6, &ccp);
    ft += assertEqualsChar(ccp.ccpa, puissance, "CAR-6A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAR-6B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAR-6C");

    return ft;
}

unsigned char test_phaseSelonHallEtDirection() {
    unsigned char ft = 0;

    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), 1, "PHD-10");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, AVANT), 2, "PHD-20");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b010, AVANT), 3, "PHD-30");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b110, AVANT), 4, "PHD-40");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b100, AVANT), 5, "PHD-50");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b101, AVANT), 6, "PHD-60");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), 1, "PHD-11");

    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), ERROR, "PHD-E0");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, AVANT), 2, "PHD-E1");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, AVANT), ERROR, "PHD-E1");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), ERROR, "PHD-E3");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b010, AVANT), 3, "PHD-E4");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, ARRIERE), 2, "PHD-E1");

    return ft;
}

/**
 * Point d'entrée pour les tests du moteur.
 * @return Nombre de tests en erreur.
 */
unsigned char test_moteur() {
    unsigned char ft = 0;

    ft += test_phaseSelonHall();
    ft += test_calculeAmplitudesArret();
    ft += test_phaseSelonHallEtDirection();

    return ft;
}

#endif