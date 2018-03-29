#include <htc.h>

#include "domaine.h"
#include "tableauDeBord.h"
#include "test.h"
#include "evenements.h"
#include "moteur.h"
#include "i2c.h"

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
    signed char step;

    if (mesureDeVitessePhase0 != 0) {
        // Établit la direction de rotation (le déplacement est toujours 1):
        step = mesureDeVitessePhase0 - phase;
        switch(step) {
            case -1:
            case 5:
                tableauDeBord.deplacementMesure.direction = AVANT;
                tableauDeBord.deplacementMesure.magnitude = 1;
                break;

            case 1:
            case -5:
                tableauDeBord.deplacementMesure.direction = ARRIERE;
                tableauDeBord.deplacementMesure.magnitude = 1;
                break;
                
            default:
                tableauDeBord.deplacementMesure.magnitude = 0;
        }
        // Incrémente la mesure de vitesse:
        opereAplusB(mesureDeVitesse, &(tableauDeBord.deplacementMesure));
    }
    mesureDeVitessePhase0 = phase;

}

void MOTEUR_machine(EvenementEtValeur *ev) {
    static MagnitudeEtDirection *tensionMoyenne;
    static MagnitudeEtDirection mesureDeVitesse = {0, AVANT};
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
            i2cExposeValeur(LECTURE_I2C_VITESSE_MESUREE, tableauDeBord.vitesseMesuree.magnitude);
            enfileMessageInterne(VITESSE_MESUREE, 0);
            break;
    }
}

#ifdef TEST
#define P 30
void test_phaseSelonHall() {
    verifieEgalite("PSH-00", ERROR, phaseSelonHall(0));
    verifieEgalite("PSH-01", 1, phaseSelonHall(0b001));
    verifieEgalite("PSH-02", 2, phaseSelonHall(0b011));
    verifieEgalite("PSH-03", 3, phaseSelonHall(0b010));
    verifieEgalite("PSH-04", 4, phaseSelonHall(0b110));
    verifieEgalite("PSH-05", 5, phaseSelonHall(0b100));
    verifieEgalite("PSH-06", 6, phaseSelonHall(0b101));
    verifieEgalite("PSH-07", ERROR, phaseSelonHall(7));
}
void test_mesureVitesseMarcheAvant() {
    unsigned char tour, phase;
    MagnitudeEtDirection mesureDeVitesse = {AVANT, 0};
    
    for (tour = 0; tour < 10; tour++) {
        for (phase = 1; phase <= 6; phase++) {
            mesureVitesse(phase, &mesureDeVitesse);
        }
    }
    verifieEgalite("MOMV_01", mesureDeVitesse.direction, AVANT);
    verifieEgalite("MOMV_02", mesureDeVitesse.magnitude, 59);
}
void test_mesureVitesseMarcheArriere() {
    unsigned char tour, phase;
    MagnitudeEtDirection mesureDeVitesse = {AVANT, 0};
    
    for (tour = 0; tour < 10; tour++) {
        for (phase = 6; phase >0; phase--) {
            mesureVitesse(phase, &mesureDeVitesse);
        }
    }
    verifieEgalite("MOMV_11", mesureDeVitesse.direction, ARRIERE);
    verifieEgalite("MOMV_12", mesureDeVitesse.magnitude, 59);
}
void test_mesureVitesseInversionMarche() {
    MagnitudeEtDirection mesureDeVitesse = {AVANT, 0};
    mesureDeVitessePhase0 = 6;
    
    mesureVitesse(1, &mesureDeVitesse);
    mesureVitesse(2, &mesureDeVitesse);
    mesureVitesse(3, &mesureDeVitesse);
    mesureVitesse(2, &mesureDeVitesse);
    mesureVitesse(1, &mesureDeVitesse);
    mesureVitesse(6, &mesureDeVitesse);
    mesureVitesse(5, &mesureDeVitesse);
    mesureVitesse(4, &mesureDeVitesse);
    
    verifieEgalite("MOMV_20a", mesureDeVitesse.direction, ARRIERE);
    verifieEgalite("MOMV_20b", mesureDeVitesse.magnitude, 2);
}
void test_mesureVitesseEtDeplacement() {
    EvenementEtValeur moteurPhase = {MOTEUR_PHASE, 0};

    moteurPhase.valeur = phaseParHall[1];
    MOTEUR_machine(&moteurPhase);
    verifieEgalite("MOVD_01", tableauDeBord.deplacementMesure.magnitude, 0);

    moteurPhase.valeur = phaseParHall[2];
    MOTEUR_machine(&moteurPhase);
    verifieEgalite("MOVD_01", tableauDeBord.deplacementMesure.magnitude, 1);
    verifieEgalite("MOVD_01", tableauDeBord.deplacementMesure.direction, AVANT);

    moteurPhase.valeur = phaseParHall[3];
    MOTEUR_machine(&moteurPhase);
    verifieEgalite("MOVD_01", tableauDeBord.deplacementMesure.magnitude, 1);
    verifieEgalite("MOVD_01", tableauDeBord.deplacementMesure.direction, AVANT);

    moteurPhase.valeur = phaseParHall[2];
    MOTEUR_machine(&moteurPhase);
    verifieEgalite("MOVD_01", tableauDeBord.deplacementMesure.magnitude, 1);
    verifieEgalite("MOVD_01", tableauDeBord.deplacementMesure.direction, ARRIERE);
}

void test_calculeAmplitudesMarcheArriere() {
    MagnitudeEtDirection tensionMoyenne = {ARRIERE, P};
    
    calculeAmplitudes(&tensionMoyenne, 1);
    verifieEgalite("PWMAR1AH", AH, 0);
    verifieEgalite("PWMAR1AL", AL, 0);
    verifieEgalite("PWMAR1BH", BH, P);
    verifieEgalite("PWMAR1BL", BL, 0);
    verifieEgalite("PWMAR1CH", CH, 0);
    verifieEgalite("PWMAR1CL", CL, 1);

    calculeAmplitudes(&tensionMoyenne, 2);
    verifieEgalite("PWMAR2AH", AH, 0);
    verifieEgalite("PWMAR2AL", AL, 1);
    verifieEgalite("PWMAR2BH", BH, P);
    verifieEgalite("PWMAR2BL", BL, 0);
    verifieEgalite("PWMAR2CH", CH, 0);
    verifieEgalite("PWMAR2CL", CL, 0);

    calculeAmplitudes(&tensionMoyenne, 3);
    verifieEgalite("PWMAR3AH", AH, 0);
    verifieEgalite("PWMAR3AL", AL, 1);
    verifieEgalite("PWMAR3BH", BH, 0);
    verifieEgalite("PWMAR3BL", BL, 0);
    verifieEgalite("PWMAR3CH", CH, P);
    verifieEgalite("PWMAR3CL", CL, 0);

    calculeAmplitudes(&tensionMoyenne, 4);
    verifieEgalite("PWMAR4AH", AH, 0);
    verifieEgalite("PWMAR4AL", AL, 0);
    verifieEgalite("PWMAR4BH", BH, 0);
    verifieEgalite("PWMAR4BL", BL, 1);
    verifieEgalite("PWMAR4CH", CH, P);
    verifieEgalite("PWMAR4CL", CL, 0);

    calculeAmplitudes(&tensionMoyenne, 5);
    verifieEgalite("PWMAR5AH", AH, P);
    verifieEgalite("PWMAR5AL", AL, 0);
    verifieEgalite("PWMAR5BH", BH, 0);
    verifieEgalite("PWMAR5BL", BL, 1);
    verifieEgalite("PWMAR5CH", CH, 0);
    verifieEgalite("PWMAR5CL", CL, 0);

    calculeAmplitudes(&tensionMoyenne, 6);
    verifieEgalite("PWMAR6AH", AH, P);
    verifieEgalite("PWMAR6AL", AL, 0);
    verifieEgalite("PWMAR6BH", BH, 0);
    verifieEgalite("PWMAR6BL", BL, 0);
    verifieEgalite("PWMAR6CH", CH, 0);
    verifieEgalite("PWMAR6CL", CL, 1);
}
void test_calculeAmplitudesMarcheAvant() {
    MagnitudeEtDirection tensionMoyenne = {AVANT, P};
        
    calculeAmplitudes(&tensionMoyenne, 1);
    verifieEgalite("PWMAV1AH", AH, 0);
    verifieEgalite("PWMAV1AL", AL, 0);
    verifieEgalite("PWMAV1BH", BH, 0);
    verifieEgalite("PWMAV1BL", BL, 1);
    verifieEgalite("PWMAV1CH", CH, P);
    verifieEgalite("PWMAV1CL", CL, 0);

    calculeAmplitudes(&tensionMoyenne, 2);
    verifieEgalite("PWMAV2AH", AH, P);
    verifieEgalite("PWMAV2AL", AL, 0);
    verifieEgalite("PWMAV2BH", BH, 0);
    verifieEgalite("PWMAV2BL", BL, 1);
    verifieEgalite("PWMAV2CH", CH, 0);
    verifieEgalite("PWMAV2CL", CL, 0);

    calculeAmplitudes(&tensionMoyenne, 3);
    verifieEgalite("PWMAV3AH", AH, P);
    verifieEgalite("PWMAV3AL", AL, 0);
    verifieEgalite("PWMAV3BH", BH, 0);
    verifieEgalite("PWMAV3BL", BL, 0);
    verifieEgalite("PWMAV3CH", CH, 0);
    verifieEgalite("PWMAV3CL", CL, 1);

    calculeAmplitudes(&tensionMoyenne, 4);
    verifieEgalite("PWMAV4AH", AH, 0);
    verifieEgalite("PWMAV4AL", AL, 0);
    verifieEgalite("PWMAV4BH", BH, P);
    verifieEgalite("PWMAV4BL", BL, 0);
    verifieEgalite("PWMAV4CH", CH, 0);
    verifieEgalite("PWMAV4CL", CL, 1);

    calculeAmplitudes(&tensionMoyenne, 5);
    verifieEgalite("PWMAV5AH", AH, 0);
    verifieEgalite("PWMAV5AL", AL, 1);
    verifieEgalite("PWMAV5BH", BH, P);
    verifieEgalite("PWMAV5BL", BL, 0);
    verifieEgalite("PWMAV5CH", CH, 0);
    verifieEgalite("PWMAV5CL", CL, 0);

    calculeAmplitudes(&tensionMoyenne, 6);
    verifieEgalite("PWMAV6AH", AH, 0);
    verifieEgalite("PWMAV6AL", AL, 1);
    verifieEgalite("PWMAV6BH", BH, 0);
    verifieEgalite("PWMAV6BL", BL, 0);
    verifieEgalite("PWMAV6CH", CH, P);
    verifieEgalite("PWMAV6CL", CL, 0);
}
void test_moteurMesureVitesse() {
    EvenementEtValeur ev = {AUCUN_EVENEMENT, 0};
    
    initialiseMessagesInternes();
    
    ev.evenement = MOTEUR_PHASE;
    
    ev.valeur = phaseParHall[1];
    MOTEUR_machine(&ev);
    ev.valeur = phaseParHall[2];
    MOTEUR_machine(&ev);
    ev.valeur = phaseParHall[3];
    MOTEUR_machine(&ev);
    
    ev.evenement = BASE_DE_TEMPS;
    MOTEUR_machine(&ev);

    verifieEgalite("MOMEVE01", tableauDeBord.vitesseMesuree.direction, AVANT);
    verifieEgalite("MOMEVE02", tableauDeBord.vitesseMesuree.magnitude, 2);
    verifieEgalite("MOMEVE03", defileMessageInterne()->evenement, VITESSE_MESUREE);
    verifieEgalite("MOMEVE04", defileMessageInterne()->evenement, 0);
}
void test_moteurTensionMoyenneEtChangementDePhase() {
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
    verifieEgalite("MTMPAH", AH, 0);
    verifieEgalite("MTMPAL", AL, 0);
    verifieEgalite("MTMPBH", BH, 0);
    verifieEgalite("MTMPBL", BL, 1);
    verifieEgalite("MTMPCH", CH, P);
    verifieEgalite("MTMPCL", CL, 0);
}

/**
 * Point d'entrée pour les tests du moteur.
 * @return Nombre de tests en erreur.
 */
void test_moteur() {
    test_phaseSelonHall();
    test_mesureVitesseMarcheAvant();
    test_mesureVitesseMarcheArriere();
    test_mesureVitesseInversionMarche();
    test_mesureVitesseEtDeplacement();
    test_calculeAmplitudesMarcheArriere();
    test_calculeAmplitudesMarcheAvant();
    
    test_moteurMesureVitesse();
    test_moteurTensionMoyenneEtChangementDePhase();
}

#endif