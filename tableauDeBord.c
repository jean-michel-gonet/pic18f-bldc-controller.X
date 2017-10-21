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
 */
void initialiseMessagesInternes() {
    fileReinitialise(&fileMessagesInternes);
}

/**
 * Réinitialise le tableau de bord et la file des messages internes.
 */
void initialiseTableauDeBord() {
    initialiseMessagesInternes();
    
    tableauDeBord.vitesseMesuree.direction = AVANT;
    tableauDeBord.vitesseMesuree.magnitude = 0;

    tableauDeBord.vitesseDemandee.direction = AVANT;
    tableauDeBord.vitesseDemandee.magnitude = 0;

    tableauDeBord.deplacementMesure.direction = AVANT;
    tableauDeBord.deplacementMesure.magnitude = 0;

    tableauDeBord.deplacementDemande.direction = AVANT;
    tableauDeBord.deplacementDemande.magnitude = 0;

    tableauDeBord.tensionMoyenne.direction = AVANT;
    tableauDeBord.tensionMoyenne.magnitude = 0;

    tableauDeBord.positionRouesAvant.tempsBas.valeur = 65535 - 3000;
    tableauDeBord.positionRouesAvant.tempsHaut.valeur = 65535 - 37000;
    
    tableauDeBord.tempsDeDeplacement = 0;
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
