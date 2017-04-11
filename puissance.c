#include "puissance.h"
#include "test.h"
#include "tableauDeBord.h"

#define TENSION_MOYENNE_MAX 180 * 32 
#define TENSION_MOYENNE_MAX_REDUITE 40 * 32
#define TENSION_ALIMENTATION_MIN 7.4
#define LECTURE_ALIMENTATION_MIN (unsigned char) (255 * (TENSION_ALIMENTATION_MIN / 2) / 5)

#undef NO_PID

typedef enum {
    /**
     * En mode déplacement, le module de puissance est piloté par la vitesse
     * demandée. L'erreur I est toujours zéro.
     */
    MODE_PID_DEPLACEMENT,
    /**
     * En mode manoeuvre, le module de puissance est piloté par l'erreur I. La
     * vitesse demandée est toujours zéro.
     */
    MODE_PID_MANOEUVRE
} ModePID;

/** 
 * La tension moyenne maximum peut varier si la tension d'alimentation
 * tombe en dessous d'un certain seuil.
 */
static int tensionMoyenneMax = TENSION_MOYENNE_MAX;

/**
 * Le mode actuel du module de puissance.
 */
static ModePID modePID = MODE_PID_DEPLACEMENT;

/**
 * En mode manoeuvre, la vitesse demandée est toujours zéro.
 */
static MagnitudeEtDirection vitesseZero = {AVANT, 0};

#define P 16
#define I 3
#define D 48

#define P_ 16
#define I_ 3
#define D_ 8

int soustraitAmoinsB(MagnitudeEtDirection *a, 
                     MagnitudeEtDirection *b) {
    int resultat;
    switch (a->direction) {
        case AVANT:
            resultat = a->magnitude;
            break;
        case ARRIERE:
            resultat = -a->magnitude;
            break;
        default:
            return 0;
    }
    switch (b->direction) {
        case ARRIERE:
            return resultat + b->magnitude;
        case AVANT:
        default:
            return resultat - b->magnitude;
    }
}

static int tensionMoyenne = 0;   // Tension moyenne, multipliée par 32
static int erreurPrecedente = 0; // Erreur précédente, pour calculer D.
static int erreurI = 0;          // Somme des erreurs précédentes, pour I.

/**
 * Réinitialise le PID.
 */
void reinitialisePid() {
    tensionMoyenne = 0;
    erreurPrecedente = 0;
    erreurI = 0;
}

/**
 * Corrige la tension moyenne du {@link TableauDeBord} selon différence observée entre
 * la vitesse mesurée et la vitesse demandée.
 * @param vitesseMesuree Dernière vitesse mesurée.
 * @param vitesseDemandee Dernière Vitesse demandée.
 */
void pidTensionMoyenne(MagnitudeEtDirection *vitesseMesuree, 
                       MagnitudeEtDirection *vitesseDemandee) {        
#ifdef NO_PID
    tableauDeBord.tensionMoyenne.direction = vitesseDemandee->direction;
    tableauDeBord.tensionMoyenne.magnitude = vitesseDemandee->magnitude;
#else
    int erreurD;
    int erreurP;
    int correction;
    int magnitude;

    // Calcule l'erreur P:
    if (modePID == MODE_PID_DEPLACEMENT) {
        erreurP = soustraitAmoinsB(vitesseDemandee, vitesseMesuree);
    } else {
        erreurP = soustraitAmoinsB(&vitesseZero, vitesseMesuree);
    }
    correction  = erreurP * P;

    // Calcule l'erreur I:
    if (modePID == MODE_PID_MANOEUVRE) {
        erreurI += erreurP;
        if (erreurI < -800) {
            erreurI = -800;
        }
        if (erreurI > 800) {
            erreurI = 800;
        }
        correction += erreurI * I;
    } else {
        erreurI = 0;
    }
    
    // Calcule l'erreur D:
    erreurD = erreurP - erreurPrecedente;
    erreurPrecedente = erreurP;
    correction += erreurD * D;
    
    // Corrige la tension moyenne:
    tensionMoyenne += correction;

    // Limite la tension moyenne:
    if (tensionMoyenne < -tensionMoyenneMax) {
        // Pour éviter d'accumuler de l'erreur si on limite la puissance.
        tensionMoyenne = -tensionMoyenneMax;
        erreurI = 0;
    }
    if (tensionMoyenne > tensionMoyenneMax) {
        tensionMoyenne = tensionMoyenneMax;
        // Pour éviter d'accumuler de l'erreur si on limite la puissance.
        erreurI = 0;
    }

    // Transfère la tension moyenne sur le tableau de bord:
    if (tensionMoyenne < 0) {
        tableauDeBord.tensionMoyenne.direction = ARRIERE;
        magnitude = -tensionMoyenne;
    } else {
        tableauDeBord.tensionMoyenne.direction = AVANT;
        magnitude = tensionMoyenne;
    }
    magnitude >>= 5;
    tableauDeBord.tensionMoyenne.magnitude = (unsigned char) magnitude;
#endif
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
    if (vitesseDemandee->magnitude < 5) {
        vitesseDemandee->magnitude = 0;
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
                if (tensionMoyenneMax > TENSION_MOYENNE_MAX_REDUITE) {
                    tensionMoyenneMax -= 32;
                }
            } else {
                if (tensionMoyenneMax < TENSION_MOYENNE_MAX) {
                    tensionMoyenneMax += 32;
                }
            }
            break;

        case VITESSE_MESUREE:
            vitesseMesuree = &(tableauDeBord.vitesseMesuree);
            pidTensionMoyenne(vitesseMesuree, &vitesseDemandee);
            enfileMessageInterne(MOTEUR_TENSION_MOYENNE);
            if (modePID == MODE_PID_MANOEUVRE) {
                if (erreurI == 0) {
                    enfileMessageInterne(DEPLACEMENT_ATTEINT);
                }
            }
            break;

        case VITESSE_DEMANDEE:
            evalueVitesseDemandee(ev->valeur, &vitesseDemandee);
            modePID = MODE_PID_DEPLACEMENT;
            break;
            
        case DEPLACEMENT_DEMANDE:
            modePID = MODE_PID_MANOEUVRE;
            erreurI = ev->valeur - 127;
            break;
    }
}

#ifdef TEST
void test_evalue_la_vitesse_demandee() {
    MagnitudeEtDirection vitesseDemandee;
    
    evalueVitesseDemandee(128 + 30, &vitesseDemandee);
    verifieEgalite("PEV01", vitesseDemandee.direction, AVANT);
    verifieEgalite("PEV02", vitesseDemandee.magnitude, 60);

    evalueVitesseDemandee(128 - 30, &vitesseDemandee);
    verifieEgalite("PEV11", vitesseDemandee.direction, ARRIERE);
    verifieEgalite("PEV12", vitesseDemandee.magnitude, 60);

    evalueVitesseDemandee(0, &vitesseDemandee);
    verifieEgalite("PEV21", vitesseDemandee.direction, ARRIERE);
    verifieEgalite("PEV22", vitesseDemandee.magnitude, 255);

    evalueVitesseDemandee(255, &vitesseDemandee);
    verifieEgalite("PEV21", vitesseDemandee.direction, AVANT);
    verifieEgalite("PEV22", vitesseDemandee.magnitude, 255);
}
void test_limite_la_tension_moyenne_maximum() {
    int n;
    EvenementEtValeur evenementEtValeur;

    reinitialisePid();

    // Marche avant:
    evenementEtValeur.evenement = VITESSE_DEMANDEE;
    evenementEtValeur.valeur = 80;
    PUISSANCE_machine(&evenementEtValeur);    

    for (n = 0; n < 1000; n++) {
        // Avertit que l'alimentation est trop basse:
        evenementEtValeur.valeur = LECTURE_ALIMENTATION_MIN - 1;
        evenementEtValeur.evenement = LECTURE_ALIMENTATION;
        PUISSANCE_machine(&evenementEtValeur);    

        // Indique que la voiture est bloquée, pour que le PID accélère:
        evenementEtValeur.evenement = VITESSE_MESUREE;
        evenementEtValeur.valeur = 0;
        PUISSANCE_machine(&evenementEtValeur);            
    }

    // La tension moyenne de sortie est à zéro:
    verifieEgalite("PMAX01", tableauDeBord.tensionMoyenne.magnitude, TENSION_MOYENNE_MAX_REDUITE/32);
}
void test_soustrait_MagnitudeEtDirection() {
    MagnitudeEtDirection a,b;

    // Signes opposés:
    a.direction = AVANT;
    b.direction = ARRIERE;
    
    a.magnitude = 100;
    b.magnitude = 50;
    verifieEgalite("SO01p", soustraitAmoinsB(&a, &b),  150);
    verifieEgalite("SO01n", soustraitAmoinsB(&b, &a), -150);
    
    a.magnitude = 50;
    b.magnitude = 100;
    verifieEgalite("SO02p", soustraitAmoinsB(&a, &b),  150);
    verifieEgalite("SO02n", soustraitAmoinsB(&b, &a), -150);

    // Signes identiques:
    a.direction = ARRIERE;
    b.direction = ARRIERE;
    
    a.magnitude = 100;
    b.magnitude = 50;
    verifieEgalite("SO03p", soustraitAmoinsB(&a, &b), -50);
    verifieEgalite("SO03n", soustraitAmoinsB(&b, &a),  50);
}
void convertitEntierEnMagnitudeEtDirection(int v, unsigned char n, MagnitudeEtDirection *md) {
    int magnitude;
    if (v < 0) {
        md->direction = ARRIERE;
        magnitude = -v;
    } else {
        md->direction = AVANT;
        magnitude = v;
    }
    magnitude >>= n;
    md->magnitude = (unsigned char) magnitude;
}

void modelePhysique(unsigned char nombreIterations) {
    EvenementEtValeur ev = {VITESSE_MESUREE, 0};
    unsigned char t, n;
    int vitesse = 0;
    
    for (n = 0; n < nombreIterations; n++) {
        PUISSANCE_machine(&ev);
        for (t = 0; t < 5; t++) {
            // La constante 3 est calculée selon le poids de la voiture, les caractéristiques
            // du moteur, le rapport des pignons du différentiel, et une constante de temps.
            vitesse += 3 * soustraitAmoinsB(&tableauDeBord.tensionMoyenne, &tableauDeBord.vitesseMesuree);
            convertitEntierEnMagnitudeEtDirection(vitesse, 5, &tableauDeBord.vitesseMesuree);
        }
    }    
}

void test_pid_atteint_la_vitesse_demandee() {
    EvenementEtValeur ev = {VITESSE_DEMANDEE, 128 + 50};
    
    reinitialisePid();
    PUISSANCE_machine(&ev);
    
    modelePhysique(100);
    verifieEgalite("PIDV01", tableauDeBord.vitesseMesuree.magnitude, 50 * 2);
}
void test_pid_atteint_la_position_demandee() {
    EvenementEtValeur ev = {DEPLACEMENT_DEMANDE, 128 + 50};
    
    reinitialisePid();
    tableauDeBord.vitesseMesuree.direction = AVANT;
    tableauDeBord.vitesseMesuree.magnitude = 10;
    PUISSANCE_machine(&ev);
    
    modelePhysique(100);

    verifieEgalite("PIDP1", erreurI, 0);
    verifieEgalite("PIDP2", tableauDeBord.vitesseMesuree.magnitude, 0);
}
void test_MOTEUR_TENSION_MOYENNE_a_chaque_VITESSE_MESUREE() {
    EvenementEtValeur evVitesseDemandee = {VITESSE_DEMANDEE, 150};
    EvenementEtValeur evVitesseMesuree = {VITESSE_MESUREE, 128};
    unsigned char n;
    
    reinitialisePid();
    PUISSANCE_machine(&evVitesseDemandee);
    for(n = 0; n < 5; n++) {
        PUISSANCE_machine(&evVitesseMesuree);
        verifieEgalite("PID_TENSM", defileMessageInterne()->evenement, MOTEUR_TENSION_MOYENNE);
    }
}
void test_DEPLACEMENT_ATTEINT_si_deplacement_atteint() {
    EvenementEtValeur evDeplacementDemande = {DEPLACEMENT_DEMANDE, 128};
    EvenementEtValeur evVitesseMesuree = {VITESSE_MESUREE, 128};
    
    reinitialisePid();
    initialiseMessagesInternes();
    PUISSANCE_machine(&evDeplacementDemande);
    erreurI = 0;
    PUISSANCE_machine(&evVitesseMesuree);
    verifieEgalite("PID_DA01", defileMessageInterne()->evenement, MOTEUR_TENSION_MOYENNE);
    verifieEgalite("PID_DA02", defileMessageInterne()->evenement, DEPLACEMENT_ATTEINT);
}

/**
 * Tests unitaires pour le calcul de tension.
 * @return Nombre de tests en erreur.
 */
void test_puissance() {
    test_soustrait_MagnitudeEtDirection();
    test_pid_atteint_la_vitesse_demandee();
    test_pid_atteint_la_position_demandee();
    test_MOTEUR_TENSION_MOYENNE_a_chaque_VITESSE_MESUREE();
    test_DEPLACEMENT_ATTEINT_si_deplacement_atteint();
    test_evalue_la_vitesse_demandee();
    test_limite_la_tension_moyenne_maximum();
}
#endif