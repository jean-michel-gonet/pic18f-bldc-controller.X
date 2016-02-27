#include "puissance.h"
#include "test.h"
#include "tableauDeBord.h"

#define TENSION_MOYENNE_MIN 10
#define TENSION_MOYENNE_MAX 180
#define TENSION_MOYENNE_MAX_REDUITE 40
#define TENSION_ALIMENTATION_MIN 7.4
#define LECTURE_ALIMENTATION_MIN 255 * (TENSION_ALIMENTATION_MIN / 2) / 5

/** 
 * La tension moyenne maximum peut varier si la tension d'alimentation
 * tombe en dessous d'un certain seuil.
 */
static char tensionMoyenneMax = TENSION_MOYENNE_MAX;

/**
 * Corrige la tension moyenne du {@link TableauDeBord} selon différence observée entre
 * la vitesse mesurée et la vitesse demandée.
 * @param vitesseMesuree Dernière vitesse mesurée.
 * @param vitesseDemandee Dernière Vitesse demandée.
 */
void corrigeTensionMoyenne(MagnitudeEtDirection *vitesseMesuree, 
                           MagnitudeEtDirection *vitesseDemandee) {
    unsigned char magnitude = vitesseDemandee->magnitude;
    tableauDeBord.tensionMoyenne.direction = vitesseDemandee->direction;
    
    
    if (magnitude > tensionMoyenneMax) {
        magnitude = tensionMoyenneMax;
    }
    
    if (magnitude < TENSION_MOYENNE_MIN) {
        magnitude = 0;
    }
    
    tableauDeBord.tensionMoyenne.magnitude = magnitude;
}

/**
 * Évalue la vitesse demandée en termes de direction et de valeur absolue.
 * @param lecture La lecture, en provenance du contrôle 
 * pertinent (potentiomètre, radio-commande, etc).
 * @return La vitesse demandée correspondante à la lecture.
 */
void evalueVitesseDemandee(unsigned char lecture, 
                           MagnitudeEtDirection *vitesseDemandee) {
    signed char v = (signed char) (lecture - 128);
    
    if (v < 0) {
        vitesseDemandee->direction = ARRIERE;
        if (v == -128) {
            vitesseDemandee->magnitude = 255;
            return;
        } else {
            vitesseDemandee->magnitude = -v;
        }
        
    } else {
        vitesseDemandee->direction = AVANT;
        if (v == 127) {
            vitesseDemandee->magnitude = 255;
            return;
        }
        vitesseDemandee->magnitude = v;
    }
    
    vitesseDemandee->magnitude <<= 1;
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
    static MagnitudeEtDirection *vitesseMesuree;
    static MagnitudeEtDirection vitesseDemandee = {INDETERMINEE, 0};
    
    switch(ev->evenement) {
        case LECTURE_ALIMENTATION:
            if (ev->valeur < LECTURE_ALIMENTATION_MIN) {
                tensionMoyenneMax = TENSION_MOYENNE_MAX_REDUITE;
            } else {
                tensionMoyenneMax = TENSION_MOYENNE_MAX;
            }
            break;
        case VITESSE_MESUREE:
            vitesseMesuree = &(tableauDeBord.vitesseMesuree);
            corrigeTensionMoyenne(vitesseMesuree, &vitesseDemandee);
            enfileMessageInterne(MOTEUR_TENSION_MOYENNE);
            break;

        case LECTURE_RC_AVANT_ARRIERE:
            evalueVitesseDemandee(ev->valeur, &vitesseDemandee);
            break;
    }
}

#ifdef TEST
unsigned test_evalueVitesseDemandee() {
    unsigned char testsEnErreur = 0;
    MagnitudeEtDirection vitesseDemandee;
    
    evalueVitesseDemandee(128 + 30, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(vitesseDemandee.direction, AVANT, "PEV01");
    testsEnErreur += assertEqualsChar(vitesseDemandee.magnitude, 60, "PEV02");

    evalueVitesseDemandee(128 - 30, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(vitesseDemandee.direction, ARRIERE, "PEV11");
    testsEnErreur += assertEqualsChar(vitesseDemandee.magnitude, 60, "PEV12");

    evalueVitesseDemandee(0, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(vitesseDemandee.direction, ARRIERE, "PEV21");
    testsEnErreur += assertEqualsChar(vitesseDemandee.magnitude, 255, "PEV22");

    evalueVitesseDemandee(255, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(vitesseDemandee.direction, AVANT, "PEV21");
    testsEnErreur += assertEqualsChar(vitesseDemandee.magnitude, 255, "PEV22");

    return testsEnErreur;
}
unsigned char test_limiteTensionMoyenneMax() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur evenementEtValeur;

    reinitialiseMessagesInternes();
    tableauDeBord.tensionMoyenne.direction = INDETERMINEE;
    tableauDeBord.tensionMoyenne.magnitude = 0;

    evenementEtValeur.evenement = LECTURE_RC_AVANT_ARRIERE;
    evenementEtValeur.valeur = 128 + 30;
    PUISSANCE_machine(&evenementEtValeur);    
    
    evenementEtValeur.valeur = 8;
    evenementEtValeur.evenement = LECTURE_ALIMENTATION;
    PUISSANCE_machine(&evenementEtValeur);    
    
    evenementEtValeur.evenement = VITESSE_MESUREE;
    PUISSANCE_machine(&evenementEtValeur);    
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, TENSION_MOYENNE_MAX_REDUITE, "PMAX03");

    evenementEtValeur.valeur = 255;
    evenementEtValeur.evenement = LECTURE_ALIMENTATION;
    PUISSANCE_machine(&evenementEtValeur);    

    evenementEtValeur.evenement = VITESSE_MESUREE;
    PUISSANCE_machine(&evenementEtValeur);    
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, 60, "PMAX13");
    
    return testsEnErreur;
}
unsigned char test_corrigeTensionMoyenne() {
    unsigned char testsEnErreur = 0;
    MagnitudeEtDirection vitesseDemandee;
    
    vitesseDemandee.direction = AVANT;
    vitesseDemandee.magnitude = 100;
    corrigeTensionMoyenne(0, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.direction, AVANT, "PCTM01");
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, 100, "PCTM02");

    vitesseDemandee.direction = ARRIERE;
    vitesseDemandee.magnitude = 100;
    corrigeTensionMoyenne(0, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.direction, ARRIERE, "PCTM11");
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, 100, "PCTM12");

    vitesseDemandee.magnitude = TENSION_MOYENNE_MAX + 1;
    corrigeTensionMoyenne(0, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, TENSION_MOYENNE_MAX, "PCTM22");
    
    vitesseDemandee.magnitude = TENSION_MOYENNE_MIN - 1;
    corrigeTensionMoyenne(0, &vitesseDemandee);
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, 0, "PCTM32");
    
    return testsEnErreur;
}

unsigned char test_calculeVitesseDemandeeSurLecturePotentiometre() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur evenementEtValeur;

    reinitialiseMessagesInternes();
    tableauDeBord.tensionMoyenne.direction = INDETERMINEE;
    tableauDeBord.tensionMoyenne.magnitude = 0;
    
    evenementEtValeur.valeur = 128 + 30;

    evenementEtValeur.evenement = LECTURE_RC_AVANT_ARRIERE;
    PUISSANCE_machine(&evenementEtValeur);    
    testsEnErreur += assertEqualsInt((int) defileMessageInterne(), 0, "PLP01");
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.direction, INDETERMINEE, "PLP02");
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, 0, "PLP03");
    
    evenementEtValeur.evenement = VITESSE_MESUREE;
    PUISSANCE_machine(&evenementEtValeur);    
    testsEnErreur += assertEqualsChar(defileMessageInterne()->evenement, MOTEUR_TENSION_MOYENNE, "PLP11");
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.direction, AVANT, "PLP12");
    testsEnErreur += assertEqualsChar(tableauDeBord.tensionMoyenne.magnitude, 60, "PLP13");
    
    return testsEnErreur;
}

/**
 * Tests unitaires pour le calcul de tension.
 * @return Nombre de tests en erreur.
 */
unsigned char test_puissance() {
    unsigned char testsEnErreur = 0;
    
    testsEnErreur += test_evalueVitesseDemandee();
    testsEnErreur += test_corrigeTensionMoyenne();
    testsEnErreur += test_limiteTensionMoyenneMax();
    testsEnErreur += test_calculeVitesseDemandeeSurLecturePotentiometre();
    
    return testsEnErreur;
}
#endif