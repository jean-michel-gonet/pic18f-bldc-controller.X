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
void initialiseMessagesInternes() {
    fileReinitialise(&fileMessagesInternes);
}

/**
 * Ajoute un événement à la file de messages internes.
 * @param evenement L'événement.
 * @param valeur Valeur associée.
 */
void enfileMessageInterne(Evenement evenement, unsigned char valeur) {
    fileEnfile(&fileMessagesInternes, evenement);
    fileEnfile(&fileMessagesInternes, valeur);
}
    
EvenementEtValeur *defileMessageInterne() {
    static struct EVENEMENT_ET_VALEUR ev;

    if (fileEstVide(&fileMessagesInternes)) {
        return 0;
    }
    
    ev.evenement = fileDefile(&fileMessagesInternes);
    ev.valeur = fileDefile(&fileMessagesInternes);
    
    return &ev;
}

#ifdef TEST

#include "test.h"

void test_enfileEtDefileUnMessageInterne() {
    EvenementEtValeur *evenementEtValeur;

    initialiseMessagesInternes();
    
    // Enfile et défile un événement:
    enfileMessageInterne(LECTURE_COURANT, 0);
    evenementEtValeur = defileMessageInterne();
    verifieEgalite("TDB-01", evenementEtValeur->evenement, LECTURE_COURANT);
    
    // Comme la pile est vide, il n'y a plus de messages:
    verifieEgalite("TDB-02", (int) defileMessageInterne(), 0);
}

void test_enfileEtDefileDeuxMessagesInternes() {
    EvenementEtValeur *evenementEtValeur;

    initialiseMessagesInternes();
    
    // Enfile et défile un événement:
    enfileMessageInterne(LECTURE_COURANT, 0);
    enfileMessageInterne(LECTURE_TEMPERATURE, 0);
    
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

    initialiseMessagesInternes();
    
    for (n = 0; n < 100; n++) {
        // Enfile deux événements:
        enfileMessageInterne(LECTURE_COURANT, 0);
        enfileMessageInterne(LECTURE_TEMPERATURE, 0);
        
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
