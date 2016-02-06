#include "domaine.h"
#include "tableauDeBord.h"


/**
 * Espace mémoire pour la file.
 */
EvenementEtValeur file[8];

/** Curseur d'entrée de la file. */
unsigned char file_entree = 0;

/** Curseur de sortie de la file. */
unsigned char file_sortie = 0;

/** Si différent de zéro, alors la file a débordé. */
unsigned char file_deborde = 0;

void enfileMessageInterne(Evenement evenement) {
    struct EVENEMENT_ET_VALEUR *ev;

    if (file_deborde == 0) {
        ev = &file[file_entree++];

        file_entree &= 7;

        if (file_entree == file_sortie) {
            file_deborde = 1;
        }

        ev->evenement = evenement;
        ev->valeur = 0;
    }
}
    
EvenementEtValeur *defileMessageInterne() {
    struct EVENEMENT_ET_VALEUR *ev;

    if (file_sortie == file_entree) {
        return 0;
    }
    ev = &file[file_sortie ++];
    file_sortie &= 7;
    
    return ev;
}
