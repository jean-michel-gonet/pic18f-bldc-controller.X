#ifndef __DOMAINE_H
#define __DOMAINE_H

// Quelques constantes:
#define ERROR 255
#define CYCLE 36
#define TEMPS_MORT 12
#define CCPR_MIN 0

enum EVENEMENT {
    /** Fin de p�riode du PWM. */
    TICTAC,

    /** Le moteur vient de changer de phase.*/
    PHASE,

    /** Il s'est ecoul� trop de temps depuis le dernier changement de phase. */
    BLOCAGE,

    /** La vitesse a �t� sp�cifi�e. */
    VITESSE
};

enum DIRECTION {
    AVANT,
    ARRIERE
};

#endif