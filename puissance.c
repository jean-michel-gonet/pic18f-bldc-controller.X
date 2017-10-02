#include "puissance.h"
#include "test.h"
#include "tableauDeBord.h"
#include "i2c.h"

#define TENSION_MOYENNE_MAX 180 * 64 
#define TENSION_MOYENNE_MAX_REDUITE 40 * 64
#define TENSION_ALIMENTATION_MIN 7.4
#define LECTURE_ALIMENTATION_MIN (unsigned char) (255 * (TENSION_ALIMENTATION_MIN / 2) / 5)

/**
 * Énumère les type de régulation PID.
 */
typedef enum {
    /**
     * En mode déplacement, la régulation s'effectue sur la distance 
     * à parcourir.
     */
    MODE_PID_DEPLACEMENT,
    /**
     * En mode vitesse, la régulation s'effectue sur la vitesse mesurée.
     */
    MODE_PID_VITESSE
} ModePid;

/** Indique le type de régulation PID à appliquer. */
ModePid modePid = MODE_PID_DEPLACEMENT;

/** 
 * La tension moyenne maximum peut varier si la tension d'alimentation
 * tombe en dessous d'un certain seuil.
 */
static int tensionMoyenneMax = TENSION_MOYENNE_MAX;

/**
 * En mode manoeuvre il s'agit toujours d'atteindre le déplacement zéro.
 */
static MagnitudeEtDirection deplacementZero = {AVANT, 0};

/** Paramètres PID. */
#define P_VITESSE 24
#define D_VITESSE 9

#define P_DEPLACEMENT 1
#define D_DEPLACEMENT 6

static int tensionMoyenne = 0;   // Tension moyenne, multipliée par 32
static int erreurPrecedente = 0; // Erreur précédente, pour calculer D.

/**
 * Réinitialise le PID.
 */
void reinitialisePid() {
    tensionMoyenne = 0;
    erreurPrecedente = 0;
}

void corrigeTensionMoyenne(int correction) {
    int magnitude;

    // Corrige la tension moyenne:
    tensionMoyenne += correction;

    // Limite la tension moyenne:
    if (tensionMoyenne < -tensionMoyenneMax) {
        tensionMoyenne = -tensionMoyenneMax;
    }
    if (tensionMoyenne > tensionMoyenneMax) {
        tensionMoyenne = tensionMoyenneMax;
    }

    // Transfère la tension moyenne sur le tableau de bord:
    if (tensionMoyenne < 0) {
        tableauDeBord.tensionMoyenne.direction = ARRIERE;
        magnitude = -tensionMoyenne;
    } else {
        tableauDeBord.tensionMoyenne.direction = AVANT;
        magnitude = tensionMoyenne;
    }
    magnitude >>= 6;
    tableauDeBord.tensionMoyenne.magnitude = (unsigned char) magnitude;
}

/**
 * Corrige la tension moyenne du {@link TableauDeBord} selon la différence 
 * observée entre la vitesse mesurée et la vitesse demandée.
 * @param vitesseMesuree Dernière vitesse mesurée.
 * @param vitesseDemandee Dernière vitesse demandée.
 */
void regulateurVitesse(MagnitudeEtDirection *vitesseMesuree, 
                       MagnitudeEtDirection *vitesseDemandee) {        
    int erreurD;
    int erreurP;
    int correction;

    // Calcule l'erreur P:
    erreurP = compareAetB(vitesseDemandee, vitesseMesuree);
    correction  = erreurP * P_VITESSE;
    
    // Calcule l'erreur D:
    erreurD = erreurP - erreurPrecedente;
    erreurPrecedente = erreurP;
    correction += erreurD * D_VITESSE;

    corrigeTensionMoyenne(correction);
}

/**
 * Corrige la tension moyenne du {@link TableauDeBord} pour réduire l'erreur
 * de déplacement à zéro.
 * @param erreurDePosition Distance encore à parcourir.
 * @param tempsDeDeplacement Temps écoulé depuis la dernière mesure de distance.
 */
void regulateurDeplacement(MagnitudeEtDirection *deplacementDemande, 
                           unsigned char tempsDeDeplacement) {
    static MagnitudeEtDirection zero = {0, AVANT};    
    const unsigned char const div[50] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 
        4, 4, 5, 5, 6, 7, 8, 10, 12, 16, 25, 50
    };
    
    int erreurD;
    int erreurP;
    int correction;

    // Calcule l'erreur P:
    erreurP = compareAetB(deplacementDemande, &zero);
    correction  = erreurP * P_DEPLACEMENT;
    
    // Calcule l'erreur D:
    if (erreurP < erreurPrecedente) {
        erreurD = -div[tempsDeDeplacement];
    } else {
        erreurD = div[tempsDeDeplacement];
    }
    erreurPrecedente = erreurP;
    correction += erreurD * D_DEPLACEMENT;
    
    // Corrige la tension moyenne:
    corrigeTensionMoyenne(correction);
}

/**
 * Machine à états pour réguler la puissance (tension moyenne) appliquée
 * au moteur.
 * @param ev Événement à traiter.
 */
void PUISSANCE_machine(EvenementEtValeur *ev) {
    static MagnitudeEtDirection magnitudeEtDirection = {AVANT, 0};
    
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
            if (modePid == MODE_PID_VITESSE) {
                regulateurVitesse(&(tableauDeBord.vitesseMesuree), 
                                  &(tableauDeBord.vitesseDemandee));
                enfileMessageInterne(MOTEUR_TENSION_MOYENNE, 0);
            }
            break;

        case MOTEUR_PHASE:
            if (modePid == MODE_PID_DEPLACEMENT) {
                if (opereAmoinsB(&(tableauDeBord.deplacementDemande), 
                             &(tableauDeBord.deplacementMesure))) {
                    enfileMessageInterne(DEPLACEMENT_ATTEINT, 0);
                }
                regulateurDeplacement(&(tableauDeBord.deplacementDemande), 
                                      tableauDeBord.tempsDeDeplacement);
                enfileMessageInterne(MOTEUR_TENSION_MOYENNE, 0);
            }
            break;

        case VITESSE_DEMANDEE:
            modePid = MODE_PID_VITESSE;
            convertitEnMagnitudeEtDirection(ev->valeur, &(tableauDeBord.vitesseDemandee));
            break;
            
        case DEPLACEMENT_DEMANDE:
            modePid = MODE_PID_DEPLACEMENT;
            convertitEnMagnitudeEtDirection(ev->valeur, &magnitudeEtDirection);
            opereAmoinsB(&(tableauDeBord.deplacementDemande), &magnitudeEtDirection);
    }
}

#ifdef TEST
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
    verifieEgalite("PMAX01", tableauDeBord.tensionMoyenne.magnitude, TENSION_MOYENNE_MAX_REDUITE/64);
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
            vitesse += 3 * compareAetB(&tableauDeBord.tensionMoyenne, &tableauDeBord.vitesseMesuree);
            convertitEntierEnMagnitudeEtDirection(vitesse, 5, &tableauDeBord.vitesseMesuree);
        }
    }    
}

void test_pid_atteint_la_vitesse_demandee() {
    EvenementEtValeur vitesseDemandee = {VITESSE_DEMANDEE, NEUTRE + 50};
    tableauDeBord.vitesseMesuree.direction = AVANT;
    tableauDeBord.vitesseMesuree.magnitude = 0;
    reinitialisePid();
    PUISSANCE_machine(&vitesseDemandee);
    
    modelePhysique(100);
    verifieEgalite("PIDV01", tableauDeBord.vitesseMesuree.magnitude, 50 * 2);
}

void test_pid_atteint_le_deplacement_demande() {
    EvenementEtValeur deplacementDemande = {DEPLACEMENT_DEMANDE, NEUTRE + 50};
    tableauDeBord.vitesseMesuree.direction = AVANT;
    tableauDeBord.vitesseMesuree.magnitude = 0;
    reinitialisePid();
    PUISSANCE_machine(&deplacementDemande);
    
    modelePhysique(100);
    verifieEgalite("PIDD01", tableauDeBord.vitesseMesuree.magnitude, 0);
    verifieEgalite("PIDD02", tableauDeBord.deplacementDemande.magnitude, 0);
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

/**
 * Tests unitaires pour le calcul de tension.
 * @return Nombre de tests en erreur.
 */
void test_puissance() {
    test_pid_atteint_la_vitesse_demandee();
    test_pid_atteint_le_deplacement_demande();
    test_MOTEUR_TENSION_MOYENNE_a_chaque_VITESSE_MESUREE();
    test_limite_la_tension_moyenne_maximum();
}
#endif