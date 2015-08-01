#ifndef __DOMAINE_H
#define __DOMAINE_H

// Quelques constantes:
#define ERROR 255
#define CYCLE 36
#define TEMPS_MORT 12
#define CCPR_MIN 0

enum EVENEMENT {
    
    /** Le moteur vient de changer de phase.*/
    PHASE,

    /** Il s'est ecoulé trop de temps depuis le dernier changement de phase. */
    BLOCAGE,

    /** La vitesse demandée a été spécifiée. */
    VITESSE_DEMANDEE,

    /** La vitesse actuelle a été mesurée. */
    VITESSE_MESUREE
};

enum DIRECTION {
    AVANT,
    ARRIERE
};

#endif