#include "file.h"
#include "domaine.h"
#include "test.h"

/**
 * Espace mémoire pour la file.
 */
struct EVENEMENT_ET_VALEUR file[8];

/** Curseur d'entrée de la file. */
unsigned char file_entree = 0;

/** Curseur de sortie de la file. */
unsigned char file_sortie = 0;

/**
 * Ajoute un événement à la file.
 * @param evenement Événement.
 * @param valeur Valeur associée.
 */
void enfileEvenement(enum EVENEMENT evenement, unsigned char valeur) {
    struct EVENEMENT_ET_VALEUR *ev;

    if (file_alerte == 0) {
        ev = &file[file_entree++];

        file_entree &= 7;

        if (file_entree == file_sortie) {
            file_alerte = ERROR;
        }

        ev->evenement = evenement;
        ev->valeur = valeur;
    }
}

/**
 * Récupère un événement de la file.
 * @return L'événement.
 */
struct EVENEMENT_ET_VALEUR *defileEvenement() {
    struct EVENEMENT_ET_VALEUR *ev;

    if (file_sortie == file_entree) {
        return 0;
    }
    ev = &file[file_sortie ++];
    file_sortie &= 7;
    
    return ev;
}

#ifdef TEST
/**
 * Tests unitaires pour la file.
 * @return Nombre de tests en erreur.
 */
unsigned char test_file() {
    unsigned char ft = 0;
    struct EVENEMENT_ET_VALEUR *ev1;
    unsigned char n;

    // Test A: file vide, puis ajout de 1 élément:
    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-A-01");
    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-A-02");

    enfileEvenement(TICTAC, 10);
    ev1 = defileEvenement();
    ft += assertEqualsChar(TICTAC, ev1->evenement, "Q-A-10");
    ft += assertEqualsChar(10, ev1->valeur, "Q-A-20");

    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-A-31");
    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-A-32");

    // Test B: file vide, puis ajout de 3 éléments:
    enfileEvenement(TICTAC, 100);
    enfileEvenement(PHASE, 110);
    enfileEvenement(BLOCAGE, 120);

    ev1 = defileEvenement();
    ft += assertEqualsChar(TICTAC, ev1->evenement, "Q-B-10");
    ft += assertEqualsChar(100, ev1->valeur, "Q-B-20");

    ev1 = defileEvenement();
    ft += assertEqualsChar(PHASE, ev1->evenement, "Q-B-11");
    ft += assertEqualsChar(110, ev1->valeur, "Q-B-21");

    ev1 = defileEvenement();
    ft += assertEqualsChar(BLOCAGE, ev1->evenement, "Q-B-12");
    ft += assertEqualsChar(120, ev1->valeur, "Q-B-22");

    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-B-31");
    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-B-32");

    // Teste plusieurs tours de file:
    for (n = 0; n < 255; n++) {
        enfileEvenement(PHASE, n);
        enfileEvenement(BLOCAGE, n);

        ev1 = defileEvenement();
        ft += assertEqualsChar(ev1->evenement, PHASE, "Q-D-01");
        ft += assertEqualsChar(ev1->valeur, n, "Q-D-02");

        ev1 = defileEvenement();
        ft += assertEqualsChar(ev1->evenement, BLOCAGE, "Q-D-11");
        ft += assertEqualsChar(ev1->valeur, n, "Q-D-12");
    }

    // Test C: Remplit la file et vérifie l'alerte:
    for(n = 0; n < 7; n++) {
        enfileEvenement(BLOCAGE, n);
    }
    ft += assertEqualsChar(file_alerte, 0, "Q-C-00");
    enfileEvenement(BLOCAGE, 10);
    ft += assertEqualsChar(file_alerte, 255, "Q-C-01");
    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-C-02");
    enfileEvenement(BLOCAGE, 10);
    ft += assertEqualsInt((int) defileEvenement(), 0, "Q-C-03");

    return ft;
}

#endif

