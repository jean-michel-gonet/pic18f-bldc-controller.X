#include <htc.h>

#include "domaine.h"
#include "tableauDeBord.h"
#include "test.h"
#include "file.h"
#include "moteur.h"

#define X 255
#define OO X
//                                Direction:                AVANT      //        ARRIERE
//                                    Phase:          1  2  3  4  5  6 //     1  2  3  4  5  6
const unsigned char const pwmAParPhase[2][7] = { {OO, X, 1, 1, X, 0, 0}, {OO, X, 0, 0, X, 1, 1} };
const unsigned char const pwmBParPhase[2][7] = { {OO, 0, 0, X, 1, 1, X}, {OO, 1, 1, X, 0, 0, X} };
const unsigned char const pwmCParPhase[2][7] = { {OO, 1, X, 0, 0, X, 1}, {OO, 0, X, 1, 1, X, 0} };

/**
 * Contient les valeurs des rapports cycliques des trois
 * modulations PWM.
 */
typedef struct {
    /** Rapport cyclique pour le connecteur de la bobine A. */
    unsigned char ccpa;
    /** Rapport cyclique pour le connecteur de la bobine B. */
    unsigned char ccpb;
    /** Rapport cyclique pour le connecteur de la bobine C. */
    unsigned char ccpc;
} CCP;


/**
 * Configure les PWM par rapport à la phase spécifiée, à la tension moyenne
 * et à la direction de rotation.
 * @param tensionMoyenne Tension moyenne à utiliser. Il est conseillé de ne pas utiliser
 * une valeur trop forte ici, pour ne pas brûler le circuit.
 */
void calculeAmplitudes(MagnitudeEtDirection *tensionMoyenne, unsigned char phase) {
    unsigned char pwm;
    Direction direction = tensionMoyenne->direction;
    unsigned char magnitude = tensionMoyenne->magnitude;
    CCP ccp;
    
    if ( (phase == 0) || (phase > 6) ) {
        ccp.ccpa = 0;
        ccp.ccpb = 0;
        ccp.ccpc = 0;
    } else {
        pwm = pwmAParPhase[direction][phase];
        ccp.ccpa = (pwm == 1 ? magnitude : pwm);

        pwm = pwmBParPhase[direction][phase];
        ccp.ccpb = (pwm == 1 ? magnitude : pwm);
        
        pwm = pwmCParPhase[direction][phase];
        ccp.ccpc = (pwm == 1 ? magnitude : pwm);
    }
    
    CCPR1L = (ccp.ccpa == X ? 0 : ccp.ccpa);
    PORTCbits.RC3 = (ccp.ccpa == 0 ? 1 : 0);

    CCPR2L = (ccp.ccpb == X ? 0 : ccp.ccpb);
    PORTCbits.RC0 = (ccp.ccpb == 0 ? 1 : 0);

    CCPR3L = (ccp.ccpc == X ? 0 : ccp.ccpc);
    PORTCbits.RC7 = (ccp.ccpc == 0 ? 1 : 0);
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
 * Détermine la phase en cours d'après les senseurs hall.
 * @param hall La valeur des senseurs hall: 0b*****ZYX
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
 * Compare la phase spécifiée avec la phase précédente, et accumule le compte
 * de phases dans {@code mesureDeVitesse}.
 * @param phase La phase actuelle.
 * @param mesureDeVitesse Pour accumuler le nombre de phases détectées.
 */
void mesureVitesse(unsigned char phase, MagnitudeEtDirection *mesureDeVitesse) {
    static unsigned char phase0 = 0;
    Direction direction;
    signed char step;

    // Etablit la direction de rotation:
    if (phase0 != 0) {
        step = phase0 - phase;
        switch(step) {
            case -1:
            case 5:
                direction = AVANT;
                break;

            case 1:
            case -5:
                direction == ARRIERE;
                break;

            case 0:
            default:
                return;
        }
    }
    phase0 = phase;

    // Compte le nombre de pas dans la mesure de vitesse:
    if (direction == mesureDeVitesse->direction) {
        mesureDeVitesse->magnitude++;
    } else {
        if (--mesureDeVitesse->magnitude == 0) {
            mesureDeVitesse->direction = direction;
        }
    }
}

void MOTEUR_machine(EvenementEtValeur *ev) {
    static MagnitudeEtDirection *tensionMoyenne;
    static MagnitudeEtDirection mesureDeVitesse = {0, INDETERMINEE};
    static unsigned char phase;

    switch(ev->evenement) {

        case MOTEUR_TENSION_MOYENNE:
            tensionMoyenne = &tableauDeBord.tensionMoyenne;
            calculeAmplitudes(tensionMoyenne, phase);
            break;

        case MOTEUR_PHASE:
            phase = phaseSelonHall(ev->valeur);
            calculeAmplitudes(tensionMoyenne, phase);
            mesureVitesse(phase, &mesureDeVitesse);
            break;

        case BASE_DE_TEMPS:
            tableauDeBord.vitesseMesuree.magnitude = mesureDeVitesse.magnitude;
            tableauDeBord.vitesseMesuree.direction = mesureDeVitesse.direction;
            mesureDeVitesse.magnitude = 0;
            enfileMessageInterne(VITESSE_MESUREE);
            break;
    }
}

#ifdef TEST

unsigned char test_machine() {
    unsigned char testsEnErreur = 0;
    
    return testsEnErreur;
}

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
    unsigned char tension = 15;
    CCP ccp;
    MagnitudeEtDirection tensionMoyenne;

    // Phase inconnue:
    calculeAmplitudes(&tensionMoyenne, 0);
    ft += assertEqualsChar(ccp.ccpa, 0, "CAA-0A");
    ft += assertEqualsChar(ccp.ccpb, 0, "CAA-0B");
    ft += assertEqualsChar(ccp.ccpc, 0, "CAA-0C");

    calculeAmplitudes(&tensionMoyenne, 7);
    ft += assertEqualsChar(ccp.ccpa, 0, "CAA-0A");
    ft += assertEqualsChar(ccp.ccpb, 0, "CAA-0B");
    ft += assertEqualsChar(ccp.ccpc, 0, "CAA-0C");

    // Marche avant:
    calculeAmplitudes(&tensionMoyenne, 1);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAV-1A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAV-1B");
    ft += assertEqualsChar(ccp.ccpc, tension, "CAV-1C");

    calculeAmplitudes(&tensionMoyenne, 2);
    ft += assertEqualsChar(ccp.ccpa, tension, "CAV-2A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAV-2B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAV-2C");

    calculeAmplitudes(&tensionMoyenne, 3);
    ft += assertEqualsChar(ccp.ccpa, tension, "CAV-3A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAV-3B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAV-3C");

    calculeAmplitudes(&tensionMoyenne, 4);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAV-4A");
    ft += assertEqualsChar(ccp.ccpb, tension, "CAV-4B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAV-4C");

    calculeAmplitudes(&tensionMoyenne, 5);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAV-5A");
    ft += assertEqualsChar(ccp.ccpb, tension, "CAV-5B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAV-5C");

    calculeAmplitudes(&tensionMoyenne, 6);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAV-6A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAV-6B");
    ft += assertEqualsChar(ccp.ccpc, tension, "CAV-6C");


    // Marche arrière:
    calculeAmplitudes(&tensionMoyenne, 1);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAR-1A");
    ft += assertEqualsChar(ccp.ccpb, tension, "CAR-1B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAR-1C");

    calculeAmplitudes(&tensionMoyenne, 2);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAR-2A");
    ft += assertEqualsChar(ccp.ccpb, tension, "CAR-2B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAR-2C");

    calculeAmplitudes(&tensionMoyenne, 3);
    ft += assertEqualsChar(ccp.ccpa,         0, "CAR-3A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAR-3B");
    ft += assertEqualsChar(ccp.ccpc, tension, "CAR-3C");

    calculeAmplitudes(&tensionMoyenne, 4);
    ft += assertEqualsChar(ccp.ccpa,         X, "CAR-4A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAR-4B");
    ft += assertEqualsChar(ccp.ccpc, tension, "CAR-4C");

    calculeAmplitudes(&tensionMoyenne, 5);
    ft += assertEqualsChar(ccp.ccpa, tension, "CAR-5A");
    ft += assertEqualsChar(ccp.ccpb,         0, "CAR-5B");
    ft += assertEqualsChar(ccp.ccpc,         X, "CAR-5C");

    calculeAmplitudes(&tensionMoyenne, 6);
    ft += assertEqualsChar(ccp.ccpa, tension, "CAR-6A");
    ft += assertEqualsChar(ccp.ccpb,         X, "CAR-6B");
    ft += assertEqualsChar(ccp.ccpc,         0, "CAR-6C");

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
    ft += test_machine();

    return ft;
}

#endif