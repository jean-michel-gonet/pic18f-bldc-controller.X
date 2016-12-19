#include <htc.h>

#include "domaine.h"
#include "tableauDeBord.h"
#include "test.h"
#include "file.h"
#include "moteur.h"

typedef struct {
    unsigned char H;
    unsigned char L;
} Bobine;

typedef struct {
    Bobine A;
    Bobine B;
    Bobine C;
} Schema;

typedef struct {
    Schema avant;
    Schema arriere;
} Pwm;

const Pwm const pwmParPhase[7] = {
    {{{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}},

     //         AVANT        //        ARRIERE
    //AH, AL, BH, BL, CH, CL // AH, AL, BH, BL, CH, CL
    {{{0, 0}, {0, 1}, {1, 0}}, {{0, 0}, {1, 0}, {0, 1}}},
    {{{1, 0}, {0, 1}, {0, 0}}, {{0, 1}, {1, 0}, {0, 0}}},
    {{{1, 0}, {0, 0}, {0, 1}}, {{0, 1}, {0, 0}, {1, 0}}},
    {{{0, 0}, {1, 0}, {0, 1}}, {{0, 0}, {0, 1}, {1, 0}}},
    {{{0, 1}, {1, 0}, {0, 0}}, {{1, 0}, {0, 1}, {0, 0}}},
    {{{0, 1}, {0, 0}, {1, 0}}, {{1, 0}, {0, 0}, {0, 1}}}
};

#define AH CCPR1L 
#define AL PORTCbits.RC3
#define BH CCPR2L
#define BL PORTCbits.RC0
#define CH CCPR3L
#define CL PORTCbits.RC7


/**
 * Configure les PWM par rapport à la phase spécifiée, à la tension moyenne
 * et à la direction de rotation.
 * @param tensionMoyenne Tension moyenne à utiliser. Il est conseillé de ne pas utiliser
 * une valeur trop forte ici, pour ne pas brûler le circuit.
 */
void calculeAmplitudes(MagnitudeEtDirection *tensionMoyenne, unsigned char phase) {
    Pwm pwm;
    unsigned char magnitude;
    
    if ( (phase == 0) || (phase > 6) ) {
        AH = 0;
        BH = 0;
        CH = 0;
    } else {
        pwm = pwmParPhase[phase];
        magnitude = tensionMoyenne->magnitude;
        switch (tensionMoyenne->direction) {
            case AVANT:
                AH = pwm.avant.A.H ? magnitude : 0;
                AL = pwm.avant.A.L;
                
                BH = pwm.avant.B.H ? magnitude : 0;
                BL = pwm.avant.B.L;
                
                CH = pwm.avant.C.H ? magnitude : 0;
                CL = pwm.avant.C.L;
                
                break;
            case ARRIERE:
                AH = pwm.arriere.A.H ? magnitude : 0;
                AL = pwm.arriere.A.L;
                
                BH = pwm.arriere.B.H ? magnitude : 0;
                BL = pwm.arriere.B.L;
                
                CH = pwm.arriere.C.H ? magnitude : 0;
                CL = pwm.arriere.C.L;
                break;
        }
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

static unsigned char mesureDeVitessePhase0 = 0;

/**
 * Compare la phase spécifiée avec la phase précédente, et accumule le compte
 * de phases dans {@param mesureDeVitesse}.
 * @param phase La phase actuelle.
 * @param mesureDeVitesse Pour accumuler le nombre de phases détectées.
 */
void mesureVitesse(unsigned char phase, MagnitudeEtDirection *mesureDeVitesse) {
    Direction direction = INDETERMINEE;
    signed char step;

    // Établit la direction de rotation:
    if (mesureDeVitessePhase0 != 0) {
        step = mesureDeVitessePhase0 - phase;
        switch(step) {
            case -1:
            case 5:
                direction = AVANT;
                break;

            case 1:
            case -5:
                direction = ARRIERE;
                break;
        }
    }

    mesureDeVitessePhase0 = phase;

    if (direction != INDETERMINEE) {
        if ( (mesureDeVitesse->direction == INDETERMINEE) || (mesureDeVitesse->magnitude == 0) ) {
            mesureDeVitesse->direction = direction;
            mesureDeVitesse->magnitude = 1;
        }
        if (direction == mesureDeVitesse->direction) {
            mesureDeVitesse->magnitude++;
        } else {
            mesureDeVitesse->magnitude--;
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
#define P 30
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
unsigned char test_mesureVitesseMarcheAvant() {
    unsigned char testsEnErreur = 0;
    unsigned char tour, phase;
    MagnitudeEtDirection mesureDeVitesse = {INDETERMINEE, 0};
    
    for (tour = 0; tour < 10; tour++) {
        for (phase = 1; phase <= 6; phase++) {
            mesureVitesse(phase, &mesureDeVitesse);
        }
    }
    assertEqualsChar(mesureDeVitesse.direction, AVANT, "MOMV_01");
    assertEqualsChar(mesureDeVitesse.magnitude, 60, "MOMV_02");
    
    return testsEnErreur;
}
unsigned char test_mesureVitesseMarcheArriere() {
    unsigned char testsEnErreur = 0;
    unsigned char tour, phase;
    MagnitudeEtDirection mesureDeVitesse = {INDETERMINEE, 0};
    
    for (tour = 0; tour < 10; tour++) {
        for (phase = 6; phase >0; phase--) {
            mesureVitesse(phase, &mesureDeVitesse);
        }
    }
    assertEqualsChar(mesureDeVitesse.direction, ARRIERE, "MOMV_11");
    assertEqualsChar(mesureDeVitesse.magnitude, 60, "MOMV_12");
    
    return testsEnErreur;
}
unsigned char test_mesureVitesseInversionMarche() {
    unsigned char testsEnErreur = 0;
    
    MagnitudeEtDirection mesureDeVitesse = {INDETERMINEE, 0};
    mesureDeVitessePhase0 = 6;
    
    mesureVitesse(1, &mesureDeVitesse);
    mesureVitesse(2, &mesureDeVitesse);
    mesureVitesse(3, &mesureDeVitesse);
    mesureVitesse(2, &mesureDeVitesse);
    mesureVitesse(1, &mesureDeVitesse);
    mesureVitesse(6, &mesureDeVitesse);
    mesureVitesse(5, &mesureDeVitesse);
    mesureVitesse(4, &mesureDeVitesse);
    
    assertEqualsChar(mesureDeVitesse.direction, ARRIERE, "MOMV_20a");
    assertEqualsChar(mesureDeVitesse.magnitude, 2, "MOMV_20b");
    
    return testsEnErreur;
}
unsigned char test_calculeAmplitudesMarcheArriere() {
    unsigned char testsEnErreur = 0;
    MagnitudeEtDirection tensionMoyenne = {ARRIERE, P};
    
    calculeAmplitudes(&tensionMoyenne, 1);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAR1AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAR1AL");
    testsEnErreur += assertEqualsChar(BH, P, "PWMAR1BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAR1BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAR1CH");
    testsEnErreur += assertEqualsChar(CL, 1, "PWMAR1CL");

    calculeAmplitudes(&tensionMoyenne, 2);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAR2AH");
    testsEnErreur += assertEqualsChar(AL, 1, "PWMAR2AL");
    testsEnErreur += assertEqualsChar(BH, P, "PWMAR2BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAR2BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAR2CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAR2CL");

    calculeAmplitudes(&tensionMoyenne, 3);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAR3AH");
    testsEnErreur += assertEqualsChar(AL, 1, "PWMAR3AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAR3BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAR3BL");
    testsEnErreur += assertEqualsChar(CH, P, "PWMAR3CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAR3CL");

    calculeAmplitudes(&tensionMoyenne, 4);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAR4AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAR4AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAR4BH");
    testsEnErreur += assertEqualsChar(BL, 1, "PWMAR4BL");
    testsEnErreur += assertEqualsChar(CH, P, "PWMAR4CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAR4CL");

    calculeAmplitudes(&tensionMoyenne, 5);
    testsEnErreur += assertEqualsChar(AH, P, "PWMAR5AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAR5AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAR5BH");
    testsEnErreur += assertEqualsChar(BL, 1, "PWMAR5BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAR5CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAR5CL");

    calculeAmplitudes(&tensionMoyenne, 6);
    testsEnErreur += assertEqualsChar(AH, P, "PWMAR6AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAR6AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAR6BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAR6BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAR6CH");
    testsEnErreur += assertEqualsChar(CL, 1, "PWMAR6CL");

    return testsEnErreur;
}
unsigned char test_calculeAmplitudesMarcheAvant() {
    unsigned char testsEnErreur = 0;
    MagnitudeEtDirection tensionMoyenne = {AVANT, P};
        
    calculeAmplitudes(&tensionMoyenne, 1);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAV1AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAV1AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAV1BH");
    testsEnErreur += assertEqualsChar(BL, 1, "PWMAV1BL");
    testsEnErreur += assertEqualsChar(CH, P, "PWMAV1CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAV1CL");

    calculeAmplitudes(&tensionMoyenne, 2);
    testsEnErreur += assertEqualsChar(AH, P, "PWMAV2AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAV2AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAV2BH");
    testsEnErreur += assertEqualsChar(BL, 1, "PWMAV2BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAV2CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAV2CL");

    calculeAmplitudes(&tensionMoyenne, 3);
    testsEnErreur += assertEqualsChar(AH, P, "PWMAV3AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAV3AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAV3BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAV3BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAV3CH");
    testsEnErreur += assertEqualsChar(CL, 1, "PWMAV3CL");

    calculeAmplitudes(&tensionMoyenne, 4);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAV4AH");
    testsEnErreur += assertEqualsChar(AL, 0, "PWMAV4AL");
    testsEnErreur += assertEqualsChar(BH, P, "PWMAV4BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAV4BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAV4CH");
    testsEnErreur += assertEqualsChar(CL, 1, "PWMAV4CL");

    calculeAmplitudes(&tensionMoyenne, 5);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAV5AH");
    testsEnErreur += assertEqualsChar(AL, 1, "PWMAV5AL");
    testsEnErreur += assertEqualsChar(BH, P, "PWMAV5BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAV5BL");
    testsEnErreur += assertEqualsChar(CH, 0, "PWMAV5CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAV5CL");

    calculeAmplitudes(&tensionMoyenne, 6);
    testsEnErreur += assertEqualsChar(AH, 0, "PWMAV6AH");
    testsEnErreur += assertEqualsChar(AL, 1, "PWMAV6AL");
    testsEnErreur += assertEqualsChar(BH, 0, "PWMAV6BH");
    testsEnErreur += assertEqualsChar(BL, 0, "PWMAV6BL");
    testsEnErreur += assertEqualsChar(CH, P, "PWMAV6CH");
    testsEnErreur += assertEqualsChar(CL, 0, "PWMAV6CL");

    return testsEnErreur;
}
unsigned char test_moteurMesureVitesse() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur ev = {AUCUN_EVENEMENT, 0};
    
    reinitialiseMessagesInternes();
    
    ev.evenement = MOTEUR_PHASE;
    
    ev.valeur = phaseParHall[1];
    MOTEUR_machine(&ev);
    ev.valeur = phaseParHall[2];
    MOTEUR_machine(&ev);
    ev.valeur = phaseParHall[3];
    MOTEUR_machine(&ev);
    
    ev.evenement = BASE_DE_TEMPS;
    MOTEUR_machine(&ev);

    testsEnErreur += assertEqualsChar(tableauDeBord.vitesseMesuree.direction, AVANT, "MOMEVE01");
    testsEnErreur += assertEqualsChar(tableauDeBord.vitesseMesuree.magnitude, 3, "MOMEVE02");
    testsEnErreur += assertEqualsChar(defileMessageInterne()->evenement, VITESSE_MESUREE, "MOMEVE03");
    testsEnErreur += assertEqualsChar(defileMessageInterne()->evenement, 0, "MOMEVE04");
            
    return testsEnErreur;
}
unsigned char test_moteurTensionMoyenneEtChangementDePhase() {
    unsigned char testsEnErreur = 0;

    EvenementEtValeur ev = {AUCUN_EVENEMENT, 0};
    
    // Changement de tension moyenne:
    ev.evenement = MOTEUR_TENSION_MOYENNE;
    tableauDeBord.tensionMoyenne.direction = AVANT;
    tableauDeBord.tensionMoyenne.magnitude = P;
    MOTEUR_machine(&ev);

    // Changement de phase:
    ev.evenement = MOTEUR_PHASE;
    ev.valeur = 1;
    MOTEUR_machine(&ev);

    // Vérifie l'état de la commutation:
    testsEnErreur += assertEqualsChar(AH, 0, "MTMPAH");
    testsEnErreur += assertEqualsChar(AL, 0, "MTMPAL");
    testsEnErreur += assertEqualsChar(BH, 0, "MTMPBH");
    testsEnErreur += assertEqualsChar(BL, 1, "MTMPBL");
    testsEnErreur += assertEqualsChar(CH, P, "MTMPCH");
    testsEnErreur += assertEqualsChar(CL, 0, "MTMPCL");
    
    return testsEnErreur;
}

/**
 * Point d'entrée pour les tests du moteur.
 * @return Nombre de tests en erreur.
 */
unsigned char test_moteur() {
    unsigned char testsEnErreur = 0;

    testsEnErreur += test_phaseSelonHall();
    testsEnErreur += test_mesureVitesseMarcheAvant();
    testsEnErreur += test_mesureVitesseMarcheArriere();
    testsEnErreur += test_mesureVitesseInversionMarche();
    testsEnErreur += test_calculeAmplitudesMarcheArriere();
    testsEnErreur += test_calculeAmplitudesMarcheAvant();
    
    testsEnErreur += test_moteurMesureVitesse();
    testsEnErreur += test_moteurTensionMoyenneEtChangementDePhase();

    return testsEnErreur;
}

#endif