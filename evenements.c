#include "evenements.h"
#include "domaine.h"
#include "file.h"
#include "test.h"

/**
 * Espace mémoire pour la file.
 */
static File fileEvenementEtValeur;

/**
 * Initialise la file d'événements.
 */
void evenementsInitialise() {
    fileReinitialise(&fileEvenementEtValeur);
}

/**
 * Ajoute un événement à la file.
 * @param evenement événement.
 * @param valeur Valeur associée.
 */
void enfileEvenement(enum EVENEMENT evenement, unsigned char valeur) {
    fileEnfile(&fileEvenementEtValeur, evenement);
    fileEnfile(&fileEvenementEtValeur, valeur);
}

/**
 * Récupère un événement de la file.
 * @return L'événement.
 */
struct EVENEMENT_ET_VALEUR *defileEvenement() {
    static struct EVENEMENT_ET_VALEUR ev;

    if (fileEstVide(&fileEvenementEtValeur)) {
        return 0;
    }

    ev.evenement = fileDefile(&fileEvenementEtValeur);
    ev.valeur = fileDefile(&fileEvenementEtValeur);
    
    return &ev;
}

/**
 * Indique si la file a débordé.
 * @return 0 tant que la file n'a pas débordé.
 */
unsigned char fileDeborde() {
    return fileEstPleine(&fileEvenementEtValeur);
}

#ifdef TEST
/**
 * Tests unitaires pour la file.
 * @return Nombre de tests en erreur.
 */
void test_evenements() {
    struct EVENEMENT_ET_VALEUR *ev1;
    unsigned char n;

    evenementsInitialise();

    // Test A: file vide, puis ajout de 1 élément:
    verifieEgalite("Q-A-01", (int) defileEvenement(), 0);
    verifieEgalite("Q-A-02", (int) defileEvenement(), 0);

    enfileEvenement(MOTEUR_PHASE, 10);
    ev1 = defileEvenement();
    verifieEgalite("Q-A-10", MOTEUR_PHASE, ev1->evenement);
    verifieEgalite("Q-A-20", 10, ev1->valeur);

    verifieEgalite("Q-A-31", (int) defileEvenement(), 0);
    verifieEgalite("Q-A-32", (int) defileEvenement(), 0);

    // Test B: file vide, puis ajout de 3 éléments:
    enfileEvenement(MOTEUR_PHASE, 100);
    enfileEvenement(VITESSE_DEMANDEE, 110);
    enfileEvenement(MOTEUR_BLOCAGE, 120);

    ev1 = defileEvenement();
    verifieEgalite("Q-B-10", MOTEUR_PHASE, ev1->evenement);
    verifieEgalite("Q-B-20", 100, ev1->valeur);

    ev1 = defileEvenement();
    verifieEgalite("Q-B-11", VITESSE_DEMANDEE, ev1->evenement);
    verifieEgalite("Q-B-21", 110, ev1->valeur);

    ev1 = defileEvenement();
    verifieEgalite("Q-B-12", MOTEUR_BLOCAGE, ev1->evenement);
    verifieEgalite("Q-B-22", 120, ev1->valeur);

    verifieEgalite("Q-B-31", (int) defileEvenement(), 0);
    verifieEgalite("Q-B-32", (int) defileEvenement(), 0);

    // Teste plusieurs tours de file:
    for (n = 0; n < 255; n++) {
        enfileEvenement(MOTEUR_PHASE, n);
        enfileEvenement(MOTEUR_BLOCAGE, n);

        ev1 = defileEvenement();
        verifieEgalite("Q-D-01", ev1->evenement, MOTEUR_PHASE);
        verifieEgalite("Q-D-02", ev1->valeur, n);

        ev1 = defileEvenement();
        verifieEgalite("Q-D-11", ev1->evenement, MOTEUR_BLOCAGE);
        verifieEgalite("Q-D-12", ev1->valeur, n);
    }

    // Test C: Remplit la file et vérifie l'alerte:
    for(n = 0; n < 4; n++) {
        enfileEvenement(MOTEUR_BLOCAGE, n);
    }
    verifieEgalite("Q-C-00", fileDeborde(), 0);
    enfileEvenement(MOTEUR_BLOCAGE, 10);
    verifieEgalite("Q-C-01", fileDeborde(), 255);
}

#endif

