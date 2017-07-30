#ifndef __DOMAINE_H
#define __DOMAINE_H

// Quelques constantes:
#define ERROR 255
#define TRUE 255
#define FALSE 0
#define CYCLE 36
#define TEMPS_MORT 12
#define CCPR_MIN 0

/**
 * Position centrale d'un potentiomètre, télécommande, etc.
 * Utilisé pour transformer des variables positives entre 0 et 255 en
 * variables signées entre -128 et +127.
 */
#define NEUTRE 128 

/**
 * Liste des événements du système.
 */
typedef enum EVENEMENT {
    /** Pas d'événement. Ne pas utiliser.*/
    AUCUN_EVENEMENT,
            
    /** L'intervalle de temps qui sert de base de calcul s'est écoulé.*/
    BASE_DE_TEMPS,
            
    /** Le moteur vient de changer de phase.*/
    MOTEUR_PHASE,

    /** Le moteur est bloqué (il s'est écoulé trop de temps depuis le dernier changement de phase). */
    MOTEUR_BLOCAGE,

    /** La tension moyenne à appliquer au moteur à changé. */
    MOTEUR_TENSION_MOYENNE,
            
            /***/
    MOTEUR_PWM,

    /** Lecture du potentiomètre.*/
    LECTURE_POTENTIOMETRE,
    
    /** Lecture de la tension d'alimentation. */
    LECTURE_ALIMENTATION,
    
    /** Lecture du courant consommé par le moteur. */
    LECTURE_COURANT,
    
    /** Lecture de la température du moteur. */
    LECTURE_TEMPERATURE,
    
    /** Lecture de l'entrée Avant/Arrière de la télécommande. */
    LECTURE_RC_AVANT_ARRIERE,
    
    /** Lecture de l'entrée Gauche/Droite de la télécommande. */
    LECTURE_RC_GAUCHE_DROITE,
    
    /** La vitesse demandée pour le moteur a été spécifiée / modifiée. */
    VITESSE_DEMANDEE,

    /** La vitesse actuelle du moteur a été mesurée. */
    VITESSE_MESUREE,
            
} Evenement;

/** Pour indiquer le signe d'une magnitude absolue.*/
typedef enum DIRECTION {
    /** La direction n'est pas connue. */
    INDETERMINEE,
    /** Marche avant. */
    AVANT,
    /** Marche arrière. */
    ARRIERE,
    /** La magnitude est signée. */
    SIGNEE,
    /** La magnitude est positive. */
    POSITIVE
} Direction;

/**
 * Groupe un événement et sa valeur associée.
 */
typedef struct EVENEMENT_ET_VALEUR {
    /** L'événement. */
    enum EVENEMENT evenement;
    /** Sa valeur associée. */
    unsigned char valeur;
} EvenementEtValeur;

/** 
 * Décrit une valeur en termes de direction et de magnitude. 
 * Sert à spécifier des vitesses ou des puissances.
 */
typedef struct {
    Direction direction;
    unsigned char magnitude;
} MagnitudeEtDirection;

int compareAetB(MagnitudeEtDirection *a, 
                     MagnitudeEtDirection *b);
unsigned char opereAmoinsB(MagnitudeEtDirection *a, 
                           MagnitudeEtDirection *b);
unsigned char opereAplusB(MagnitudeEtDirection *a, 
                          MagnitudeEtDirection *b);

#ifdef TEST
void test_domaine();
#endif

#endif