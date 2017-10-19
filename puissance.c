#ifdef TEST
#include <math.h>
#endif

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
#define D_DEPLACEMENT 10

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

int correctionDeplacement = 0;

void initialiseRegulateurDeDeplacement(unsigned char valeur) {
    MagnitudeEtDirection magnitudeEtDirection;
    convertitEnMagnitudeEtDirection(valeur, &magnitudeEtDirection);
    opereAmoinsB(&(tableauDeBord.deplacementDemande), &magnitudeEtDirection);
    switch (tableauDeBord.deplacementDemande.direction) {
        case ARRIERE:
            erreurPrecedente = - tableauDeBord.deplacementDemande.magnitude;
            break;
        case AVANT:
            erreurPrecedente = tableauDeBord.deplacementDemande.magnitude;
            break;
    }
}

/**
 * Corrige la tension moyenne du {@link TableauDeBord} pour réduire l'erreur
 * de déplacement à zéro.
 * @param erreurDePosition Distance encore à parcourir.
 * @param tempsDeDeplacement Temps écoulé depuis la dernière mesure de distance.
 * @return 0 tant que le déplacement demandé n'est pas atteint.
 */
unsigned char regulateurDeplacement(MagnitudeEtDirection *deplacementMesure, 
                           unsigned char tempsDeDeplacement) {
    static const unsigned char const div[255] = {
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
        1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
        2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 
        6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12, 13, 
        14, 15, 15, 17, 18, 19, 21, 23, 25, 28, 31, 36, 42, 51, 63, 85, 127, 
        255
    };
    int correction;
    
    // Calcule la correction P:
    if (deplacementMesure->magnitude == 1) {
        switch (deplacementMesure->direction) {
            case ARRIERE:
                erreurPrecedente--;
                correction = -D_DEPLACEMENT * div[tempsDeDeplacement];
                break;
            case AVANT:
                erreurPrecedente++;
                correction = +D_DEPLACEMENT * div[tempsDeDeplacement];
                break;
        }
    } else {
        correction = 0;
    }
    correction += erreurPrecedente * P_DEPLACEMENT;

    // Corrige la tension moyenne:
    corrigeTensionMoyenne(correction);
    
    // Met à jour le déplacement
    return opereAplusB(&(tableauDeBord.deplacementDemande), 
                             &(tableauDeBord.deplacementMesure));
}

/**
 * Machine à états pour réguler la puissance (tension moyenne) appliquée
 * au moteur.
 * @param ev Événement à traiter.
 */
void PUISSANCE_machine(EvenementEtValeur *ev) {
    
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
            
        case DEPLACEMENT_ARRETE:
            if (modePid == MODE_PID_DEPLACEMENT) {
                regulateurDeplacement(&(tableauDeBord.deplacementMesure), 0);
                enfileMessageInterne(MOTEUR_TENSION_MOYENNE, 0);
            }
            break;
            
        case MOTEUR_PHASE:
            if (modePid == MODE_PID_DEPLACEMENT) {
                if (regulateurDeplacement(&(tableauDeBord.deplacementMesure), 
                                      tableauDeBord.tempsDeDeplacement)) {
                    enfileMessageInterne(DEPLACEMENT_ATTEINT, 0);
                } else {
                    enfileMessageInterne(MOTEUR_TENSION_MOYENNE, 0);
                }
            }
            break;

        case VITESSE_DEMANDEE:
            modePid = MODE_PID_VITESSE;
            convertitEnMagnitudeEtDirection(ev->valeur, &(tableauDeBord.vitesseDemandee));
            break;
            
        case DEPLACEMENT_DEMANDE:
            modePid = MODE_PID_DEPLACEMENT;
            initialiseRegulateurDeDeplacement(ev->valeur);
            break;
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

/**
 * Calcule le temps qu'un mobile soumis à l'accélération et la vitesse
 * indiquées met à parcourir la distance indiquée.
 * @param acceleration Accélération.
 * @param vitesse Vitesse.
 * @param distance Distance à parcourir.
 * @return Temps à parcourir la distance.
 */
float calculeTempsDePhase(float acceleration, float vitesse, float distance) {
    float s;
    
	// Change le signe de la distance pour qu'il soit le même 
	// que celui de la vitesse:
	if (vitesse < 0) {
		distance = -fabs(distance);
	} else {
		distance = fabs(distance);
    }
    
	// Si l'accélération et la vitesse sont négligeables 
	// par rapport à la distance:
	if ( (fabs(acceleration) < fabs(distance / 100)) && (fabs(vitesse) < fabs(distance / 100)) ) {
		return 100;
	}
		
	// Si l'accélération est négligeable
	// par rapport à la vitesse:
	if (fabs(acceleration) < fabs(vitesse / 100)) {
		return distance / vitesse;
    }

	// Si la vitesse est négligeable
	// par rapport à l'accélération:
	if  ( fabs(vitesse) < fabs(acceleration / 100)) {
		return sqrt(2 * fabs(distance) / fabs(acceleration));
    }

	// Si la voiture change de direction avant de dépasser
	// la distance de phase
	s = vitesse * vitesse + 2 * acceleration * distance;
	if (s < 0) {
		return -2 * vitesse / acceleration;
	} else {
		// Résout l'équation quadratique en choisissant
		// la bonne solution:
		if (vitesse >= 0) {
			return (-vitesse + sqrt(s)) / acceleration;
		} else {
			return (-vitesse - sqrt(s)) / acceleration;
		}
	}
}

void test_pid_atteint_le_deplacement_demande() {
    EvenementEtValeur deplacementDemande = {DEPLACEMENT_DEMANDE, NEUTRE + 50};
    EvenementEtValeur deplacementArrete = {DEPLACEMENT_ARRETE, 0};
    EvenementEtValeur moteurPhase = {MOTEUR_PHASE, 0};
    int n;
    float alpha, omega, delta;
    float tau;
    float tp;
    float nt, ntt;
    float u;

    reinitialisePid();

    tableauDeBord.vitesseMesuree.direction = AVANT;
    tableauDeBord.vitesseMesuree.magnitude = 0;
    tableauDeBord.vitesseMesuree.direction = AVANT;
    tableauDeBord.tensionMoyenne.magnitude = 0;
    tableauDeBord.deplacementDemande.direction = AVANT;
    tableauDeBord.deplacementDemande.magnitude = 0;

    PUISSANCE_machine(&deplacementDemande);
    verifieEgalite("PIDD01", tableauDeBord.deplacementDemande.magnitude, 2 * 50);

    omega = 0;
    
    for (n = 0; n < 100; n++) {
        delta = 0;
        nt = 255;
        ntt = 255.0 / 1584.1;
        do {
            u = 7.2 * tableauDeBord.tensionMoyenne.magnitude / 255.0;

            tau = 0.0613 * (u - omega / 230.4);
            alpha = tau * 10333.2;
            tp = calculeTempsDePhase(alpha, omega, 1.041) / 5.0;
            if (tp > ntt) {
                tp = ntt;
            }
            delta += tp * tp * alpha / 2.0 + tp * omega;
            omega += alpha * tp;

            ntt -= tp;
            if (ntt <= 0) {
                tableauDeBord.deplacementMesure.magnitude = 0;
                PUISSANCE_machine(&deplacementArrete);
                ntt = 255.0 / 1584.1;
            }            
            if (nt > 0) {
                nt -= tp * 1584.1;
            } 
        } while (fabs(delta) < 1.041);
        
        tableauDeBord.deplacementMesure.magnitude = 1;
        if (delta < 0) {
            tableauDeBord.deplacementMesure.direction = ARRIERE;
        } else {
            tableauDeBord.deplacementMesure.direction = AVANT;
        }
        tableauDeBord.tempsDeDeplacement = (unsigned char) nt;
        PUISSANCE_machine(&moteurPhase);
    }
    verifieEgalite("PIDD02", tableauDeBord.vitesseMesuree.magnitude, 0);
    verifieEgalite("PIDD03", tableauDeBord.deplacementDemande.magnitude, 0);
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