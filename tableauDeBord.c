#include "domaine.h"
#include "file.h"
#include "tableauDeBord.h"
#include "test.h"

/**
 * Espace mémoire pour la file.
 */
static File fileMessagesInternes;

/**
 * Réinitialise la file des messages internes.
 * Utilisée pour les tests unitaires.
 */
void reinitialiseMessagesInternes() {
    fileReinitialise(&fileMessagesInternes);
}

/**
 * Ajoute un événement à la file de messages internes.
 * @param evenement L'événement.
 */
void enfileMessageInterne(Evenement evenement) {
    fileEnfile(&fileMessagesInternes, evenement);
}
    
EvenementEtValeur *defileMessageInterne() {
    static struct EVENEMENT_ET_VALEUR ev;

    if (fileEstVide(&fileMessagesInternes)) {
        return 0;
    }
    
    ev.evenement = fileDefile(&fileMessagesInternes);
    ev.valeur = 0;
    
    return &ev;
}

#ifdef TEST

#include "test.h"

void test_enfileEtDefileUnMessageInterne() {
    EvenementEtValeur *evenementEtValeur;

    reinitialiseMessagesInternes();
    
    // Enfile et défile un événement:
    enfileMessageInterne(LECTURE_COURANT);
    evenementEtValeur = defileMessageInterne();
    verifieEgalite("TDB-01", evenementEtValeur->evenement, LECTURE_COURANT);
    
    // Comme la pile est vide, il n'y a plus de messages:
    verifieEgalite("TDB-02", (int) defileMessageInterne(), 0);
}

void test_enfileEtDefileDeuxMessagesInternes() {
    EvenementEtValeur *evenementEtValeur;

    reinitialiseMessagesInternes();
    
    // Enfile et défile un événement:
    enfileMessageInterne(LECTURE_COURANT);
    enfileMessageInterne(LECTURE_TEMPERATURE);
    
    evenementEtValeur = defileMessageInterne();
    verifieEgalite("TDB2-01", evenementEtValeur->evenement, LECTURE_COURANT);
    
    evenementEtValeur = defileMessageInterne();
    verifieEgalite("TDB2-02", evenementEtValeur->evenement, LECTURE_TEMPERATURE);
    
    // Comme la file est vide, il n'y a plus de messages:
    verifieEgalite("TDB2-03", (int) defileMessageInterne(), 0);
}

void test_enfileEtDefileUnBonPaquetDeMessages() {
    EvenementEtValeur *evenementEtValeur;
    unsigned char n;

    reinitialiseMessagesInternes();
    
    for (n = 0; n < 100; n++) {
        // Enfile deux événements:
        enfileMessageInterne(LECTURE_COURANT);
        enfileMessageInterne(LECTURE_TEMPERATURE);
        
        // Défile les événements:
        evenementEtValeur = defileMessageInterne();
        verifieEgalite("TDBP-01", evenementEtValeur->evenement, LECTURE_COURANT);
        evenementEtValeur = defileMessageInterne();
        verifieEgalite("TDBP-02", evenementEtValeur->evenement, LECTURE_TEMPERATURE);
        
        // Comme la pile est vide, il n'y a plus de messages:
        verifieEgalite("TDBP-03", (int) defileMessageInterne(), 0);
    }   
}

void test_tableauDeBord() {
    test_enfileEtDefileUnMessageInterne();
    test_enfileEtDefileDeuxMessagesInternes();
    test_enfileEtDefileUnBonPaquetDeMessages();
}

#endif
