#include "domaine.h"
#include "tableauDeBord.h"

/**
 * Espace mémoire pour la file.
 */
static EvenementEtValeur messagesInternesFile[8];

/** Curseur d'entrée de la file. */
static unsigned char messagesInternesFileEntree = 0;

/** Curseur de sortie de la file. */
static unsigned char messagesInternesFileSortie = 0;

/** Si différent de zéro, alors la file a débordé. */
static unsigned char messagesInternesFileDeborde = 0;

/**
 * Réinitialise la file des messages internes.
 * Utilisée pour les tests unitaires.
 */
void reinitialiseMessagesInternes() {
    messagesInternesFileEntree = 0;
    messagesInternesFileSortie = 0;
    messagesInternesFileDeborde = 0;
}

/**
 * Ajoute un événement à la file de messages internes.
 * @param evenement L'événement.
 */
void enfileMessageInterne(Evenement evenement) {
    struct EVENEMENT_ET_VALEUR *ev;

    if (messagesInternesFileDeborde == 0) {
        ev = &messagesInternesFile[messagesInternesFileEntree++];

        messagesInternesFileEntree &= 7;

        if (messagesInternesFileEntree == messagesInternesFileSortie) {
            messagesInternesFileDeborde = 1;
        }

        ev->evenement = evenement;
        ev->valeur = 0;
    }
}
    
EvenementEtValeur *defileMessageInterne() {
    struct EVENEMENT_ET_VALEUR *ev;

    if (messagesInternesFileSortie == messagesInternesFileEntree) {
        return 0;
    }
    ev = &messagesInternesFile[messagesInternesFileSortie ++];
    messagesInternesFileSortie &= 7;
    
    return ev;
}

#ifdef TEST

#include "test.h"

unsigned char test_enfileEtDefileUnMessageInterne() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur *evenementEtValeur;

    reinitialiseMessagesInternes();
    
    // Enfile et défile un événement:
    enfileMessageInterne(LECTURE_COURANT);
    evenementEtValeur = defileMessageInterne();
    testsEnErreur += assertEqualsChar(evenementEtValeur->evenement, LECTURE_COURANT, "TDB-01");
    
    // Comme la pile est vide, il n'y a plus de messages:
    testsEnErreur += assertEqualsInt((int) defileMessageInterne(), 0, "TDB-02");
    
    return testsEnErreur;
}

unsigned char test_enfileEtDefileDeuxMessagesInternes() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur *evenementEtValeur;

    reinitialiseMessagesInternes();
    
    // Enfile et défile un événement:
    enfileMessageInterne(LECTURE_COURANT);
    enfileMessageInterne(LECTURE_TEMPERATURE);
    
    evenementEtValeur = defileMessageInterne();
    testsEnErreur += assertEqualsChar(evenementEtValeur->evenement, LECTURE_COURANT, "TDB2-01");
    
    evenementEtValeur = defileMessageInterne();
    testsEnErreur += assertEqualsChar(evenementEtValeur->evenement, LECTURE_TEMPERATURE, "TDB2-02");
    
    // Comme la pile est vide, il n'y a plus de messages:
    testsEnErreur += assertEqualsInt((int) defileMessageInterne(), 0, "TDB2-03");
    
    return testsEnErreur;    
}

unsigned char test_enfileEtDefileUnBonPaquetDeMessages() {
    unsigned char testsEnErreur = 0;
    EvenementEtValeur *evenementEtValeur;
    unsigned char n;

    reinitialiseMessagesInternes();
    
    for (n = 0; n < 100; n++) {
        // Enfile deux événements:
        enfileMessageInterne(LECTURE_COURANT);
        enfileMessageInterne(LECTURE_TEMPERATURE);
        
        // Défile les événements:
        evenementEtValeur = defileMessageInterne();
        testsEnErreur += assertEqualsChar(evenementEtValeur->evenement, LECTURE_COURANT, "TDBP-01");
        evenementEtValeur = defileMessageInterne();
        testsEnErreur += assertEqualsChar(evenementEtValeur->evenement, LECTURE_TEMPERATURE, "TDBP-02");
        
        // Comme la pile est vide, il n'y a plus de messages:
        testsEnErreur += assertEqualsInt((int) defileMessageInterne(), 0, "TDBP-03");
        
        if (testsEnErreur > 0) {
            break;
        }
    }
    
    return testsEnErreur;    
    
}

unsigned char test_laFileDeborde() {
    unsigned char testsEnErreur = 0;

    reinitialiseMessagesInternes();
    
    enfileMessageInterne(LECTURE_COURANT);
    enfileMessageInterne(LECTURE_COURANT);
    enfileMessageInterne(LECTURE_COURANT);
    enfileMessageInterne(LECTURE_COURANT);
    enfileMessageInterne(LECTURE_COURANT);
    enfileMessageInterne(LECTURE_COURANT);
    
    enfileMessageInterne(LECTURE_COURANT);
    testsEnErreur += assertEqualsChar(messagesInternesFileDeborde, 0, "TDBD-01");
    
    enfileMessageInterne(LECTURE_COURANT);
    testsEnErreur += assertEqualsChar(messagesInternesFileDeborde, 1, "TDBD-01");
    
    return testsEnErreur;        
}


unsigned char test_tableauDeBord() {
    unsigned char testsEnErreur = 0;
    
    testsEnErreur += test_enfileEtDefileUnMessageInterne();
    testsEnErreur += test_enfileEtDefileDeuxMessagesInternes();
    testsEnErreur += test_enfileEtDefileUnBonPaquetDeMessages();
    testsEnErreur += test_laFileDeborde();
    
    return testsEnErreur;
}

#endif
