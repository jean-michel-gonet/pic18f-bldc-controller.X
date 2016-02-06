#include "puissance.h"
#include "test.h"
#include "tableauDeBord.h"

/**
 * Calcule la tension moyenne à appliquer selon différence observée entre
 * la vitesse mesuree et la vitesse demandée.
 * @param vitesseMesuree Dernière vitesse mesurée.
 * @param vitesseDemandee Dernière Vitesse demandée.
 * @return Tension moyenne à appliquer sur le moteur.
 */
MagnitudeEtDirection *calculeTensionMoyenne(MagnitudeEtDirection *vitesseMesuree, 
                                            MagnitudeEtDirection *vitesseDemandee) {
    static MagnitudeEtDirection tensionMoyenne;
    tensionMoyenne.direction = vitesseDemandee->direction;
    tensionMoyenne.magnitude = vitesseDemandee->magnitude;
    return &tensionMoyenne;
}

#define VITESSE_MAX 180
/**
 * Évalue la vitesse demandée en termes de direction et de valeur absolue.
 * @param lecture La lecture, en provenance du contrôle 
 * pertinent (potentiomètre, radiocommande, etc).
 * @return La vitesse demandée correspondante à la lecture.
 */
MagnitudeEtDirection *evalueVitesseDemandee(unsigned char lecture) {
    static MagnitudeEtDirection vitesse;
    signed char v = (signed char) lecture;
    
    if (v < 0) {
        vitesse.direction = ARRIERE;
        vitesse.magnitude = -v;
    } else {
        vitesse.direction = AVANT;
        vitesse.magnitude = v;
    }

    vitesse.magnitude <<= 1;
    if (vitesse.magnitude > VITESSE_MAX) {
        vitesse.magnitude = VITESSE_MAX;
    }

    return &vitesse;
}

typedef enum {
    ARRET,
    MARCHE,
    FREINAGE
} EtatsMachinePuissance;

#define VITESSE_DEMARRAGE 25
#define VITESSE_ARRET 10

/**
 * Machine à états pour réguler la puissance (tension moyenne) appliquée
 * au moteur.
 * @param ev Événement à traiter.
 */
void PUISSANCE_machine(EvenementEtValeur *ev) {
    static EtatsMachinePuissance etat = ARRET;
    static MagnitudeEtDirection *vitesseMesuree, *vitesseDemandee;
    MagnitudeEtDirection *tensionMoyenne;
    
    switch (etat) {
        case ARRET:
            switch(ev->evenement) {
                case VITESSE_MESUREE:
                    break;

                case LECTURE_POTENTIOMETRE:
                    vitesseDemandee = evalueVitesseDemandee(ev->valeur);
                    if (vitesseDemandee->magnitude > VITESSE_ARRET) {
                        tensionMoyenne = 
                                calculeTensionMoyenne(0, vitesseDemandee);
                        enfileMessageInterne(MOTEUR_TENSION_MOYENNE);
                        etat = MARCHE;
                    }
                    break;
            }
            break;

        case MARCHE:
            switch(ev->evenement) {
                case VITESSE_MESUREE:
                    vitesseMesuree = &(tableauDeBord.vitesseMesuree);
                    tensionMoyenne = 
                            calculeTensionMoyenne(vitesseMesuree, vitesseDemandee);
                    enfileMessageInterne(MOTEUR_TENSION_MOYENNE);
                    break;

                case LECTURE_POTENTIOMETRE:
                    vitesseDemandee = evalueVitesseDemandee(ev->valeur);
                    if (vitesseDemandee->direction != vitesseMesuree->direction) {
                        etat = FREINAGE;
                    }
                    if (vitesseDemandee->magnitude < VITESSE_ARRET) {
                        etat = FREINAGE;
                    }
                    break;
            }
            break;
        case FREINAGE:
            switch(ev->evenement) {
                case VITESSE_MESUREE:                    
                    vitesseMesuree = &(tableauDeBord.vitesseMesuree);
                    if (vitesseMesuree->magnitude == 0) {
                        etat = ARRET;
                    }
                    tensionMoyenne = calculeTensionMoyenne(vitesseMesuree, 0);
                    break;
            }
            break;
    }
}

#ifdef TEST

unsigned char test_evalueVitesse() {
    unsigned char testsEnErreur = 0;
    MagnitudeEtDirection *vitesse;
    
    vitesse = evalueVitesseDemandee(0);
    testsEnErreur += assertEqualsChar(vitesse->magnitude, 0, "VD01");

    vitesse = evalueVitesseDemandee(128);
    testsEnErreur += assertEqualsChar(vitesse->magnitude, 1, "VD02m");
    testsEnErreur += assertEqualsChar(vitesse->direction, ARRIERE, "VD02d");

    vitesse = evalueVitesseDemandee(1);
    testsEnErreur += assertEqualsChar(vitesse->magnitude, 1, "VD03m");
    testsEnErreur += assertEqualsChar(vitesse->direction, AVANT, "VD03d");

    vitesse = evalueVitesseDemandee(255);
    testsEnErreur += assertEqualsChar(vitesse->magnitude, 127, "VD04m");
    testsEnErreur += assertEqualsChar(vitesse->direction, ARRIERE, "VD04d");

    vitesse = evalueVitesseDemandee(127);
    testsEnErreur += assertEqualsChar(vitesse->magnitude, 127, "VD05m");
    testsEnErreur += assertEqualsChar(vitesse->direction, AVANT, "VD05d");
    
    return testsEnErreur;
}

unsigned char test_calculeTensionMoyenne() {
    unsigned char testsEnErreur = 0;
    MagnitudeEtDirection *tensionMoyenne;
    MagnitudeEtDirection vitesseDemandee;
    
    vitesseDemandee.direction = AVANT;
    vitesseDemandee.magnitude = 10;
    tensionMoyenne = calculeTensionMoyenne(0, &vitesseDemandee);    
    testsEnErreur += assertEqualsChar(tensionMoyenne->direction, AVANT, "CTM01");
    testsEnErreur += assertEqualsChar(tensionMoyenne->magnitude, 10, "CTM01a");
    
    vitesseDemandee.direction = AVANT;
    vitesseDemandee.magnitude = 100;
    tensionMoyenne = calculeTensionMoyenne(0, &vitesseDemandee);    
    testsEnErreur += assertEqualsChar(tensionMoyenne->direction, AVANT, "CTM02");
    testsEnErreur += assertEqualsChar(tensionMoyenne->magnitude, 100, "CTM02a");
    
    vitesseDemandee.direction = ARRIERE;
    vitesseDemandee.magnitude = 10;
    tensionMoyenne = calculeTensionMoyenne(0, &vitesseDemandee);    
    testsEnErreur += assertEqualsChar(tensionMoyenne->direction, ARRIERE, "CTM03");
    testsEnErreur += assertEqualsChar(tensionMoyenne->magnitude, 10, "CTM03a");
    
    vitesseDemandee.direction = ARRIERE;
    vitesseDemandee.magnitude = 100;
    tensionMoyenne = calculeTensionMoyenne(0, &vitesseDemandee);    
    testsEnErreur += assertEqualsChar(tensionMoyenne->direction, ARRIERE, "CTM04");
    testsEnErreur += assertEqualsChar(tensionMoyenne->magnitude, 100, "CTM04a");
    
    return testsEnErreur;
}

unsigned char test_machinePuissance() {
    unsigned char testsEnErreur = 0;

 
    return testsEnErreur;
}

/**
 * Tests unitaires pour le calcul de tension.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance() {
    unsigned char testsEnErreur = 0;
    
    testsEnErreur += test_evalueVitesse();
    testsEnErreur += test_calculeTensionMoyenne();
    testsEnErreur += test_machinePuissance();
    
    return testsEnErreur;
}
#endif