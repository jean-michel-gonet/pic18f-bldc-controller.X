#include "test.h"
#include "tableauDeBord.h"
#include "direction.h"
#include "evenements.h"
#include "i2c.h"

/** 
 * Comptes de la base de temps jusqu'à considérer 
 * que la télécommande n'est plus active. 
 */
#define TEMPS_INACTIVITE_TELECOMMANDE 1000

/** 
 * Seuil en dessous du quel on considère que la télécommande est neutre.
 */
#define SEUIL_NEUTRALITE_TELECOMMANDE 10

typedef enum {
    /** C'est la télécommande qui commande.*/
    MODE_TELECOMMANDE,
            
    /** Mode autonome; on fait ce qu'on veut.*/
    MODE_BUS_DE_COMMANDES
} BusOuTelecommande;

/** Indique qui commande.*/
BusOuTelecommande busOuTelecommande = MODE_TELECOMMANDE;

/** 
 * Nombre d'impulsions de la base de temps jusqu'à considérer
 * que la télécommande n'est plus active.
 */
unsigned int tempsInactiviteTelecommande = TEMPS_INACTIVITE_TELECOMMANDE;

/**
 * Calcule la forme du signal PWM à produire pour positionner les roues avant
 * à la position indiquée.
 * @param position Position des roues avant, entre 0 et 255. 128 est neutre.
 */
void calculePwmServoRouesAvant(unsigned char position) {
    unsigned int x = position;
    x <<= 3;
    x += 2000;
    tableauDeBord.positionRouesAvant.tempsBas.valeur = 25535 + x;
    tableauDeBord.positionRouesAvant.tempsHaut.valeur = (unsigned int) 65535 - x;
}

/**
 * Reçoit les commandes d'écriture en provenance du bus I2C.
 * Ignore les commandes si c'est la télécommande qui a le contrôle.
 * @param adresse Adresse associée à la commande.
 * @param valeur Valeur associée à la commande.
 */
void receptionBus(unsigned char adresse, unsigned char valeur) {
    if (busOuTelecommande == MODE_BUS_DE_COMMANDES) {
        switch(adresse) {
            case 0:
                enfileEvenement(LECTURE_RC_AVANT_ARRIERE, valeur);    
                break;
            case 1:
                enfileEvenement(LECTURE_RC_GAUCHE_DROITE, valeur);    
                break;
            case 2:
                break;
            case 3:
                break;
        }
    }
}

/**
 * Reçoit la lecture d'un canal de la télécommande.
 * @param evenement Le canal de télécommande.
 * @param valeur La valeur lue
 */
void receptionTelecommande(Evenement evenement, unsigned char valeur) {
    switch(busOuTelecommande) {
        case MODE_BUS_DE_COMMANDES:
            if ((valeur >= 127 + SEUIL_NEUTRALITE_TELECOMMANDE) ||
                    (valeur <= 127 - SEUIL_NEUTRALITE_TELECOMMANDE)) {
                busOuTelecommande = MODE_TELECOMMANDE;
            } else {
                busOuTelecommande = MODE_BUS_DE_COMMANDES;
            }
            break;
        case MODE_TELECOMMANDE:
            tempsInactiviteTelecommande = TEMPS_INACTIVITE_TELECOMMANDE;
            enfileEvenement(evenement, valeur);    
            break;
    }    
}

/**
 * Reçoit les lectures du canal 1 (avant / arrière) de la télécommande.
 * @param valeur Entre 0 et 255. 128 pour tout droit.
 */
void receptionTelecommandeAvantArriere(unsigned char valeur) {
    i2cExposeValeur(0, valeur);
    receptionTelecommande(LECTURE_RC_AVANT_ARRIERE, valeur);
}

/**
 * Reçoit les lectures du canal 2 (gauche / droite) de la télécommande.
 * @param valeur Entre 0 et 255. 128 pour repos.
 */
void receptionTelecommandeGaucheDroite(unsigned char valeur) {
    i2cExposeValeur(1, valeur);
    receptionTelecommande(LECTURE_RC_GAUCHE_DROITE, valeur);    
}

/**
 * Machine à états pour gérer la direction de la voiture.
 * @param ev Événement à traiter.
 */
void DIRECTION_machine(EvenementEtValeur *ev) {
    if (ev->evenement == BASE_DE_TEMPS) {
        if (tempsInactiviteTelecommande > 0) {
            tempsInactiviteTelecommande--;
            if (tempsInactiviteTelecommande == 0) {
                busOuTelecommande = MODE_BUS_DE_COMMANDES;
            }
        } else {
            busOuTelecommande = MODE_TELECOMMANDE;            
        }
    }
}

#ifdef TEST
unsigned calcule_pwm_servo_roues_avant() {
    unsigned char testsEnErreur = 0;
    
    calculePwmServoRouesAvant(128);
    verifieEgalite("DIR01", tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 3024);
    verifieEgalite("DIR01a", tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 3024);

    calculePwmServoRouesAvant(0);
    verifieEgalite("DIR11", tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 2000);
    verifieEgalite("DIR11a", tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 2000);

    calculePwmServoRouesAvant(255);
    verifieEgalite("DIR21", tableauDeBord.positionRouesAvant.tempsBas.valeur, 65535 - 40000 + 4040);
    verifieEgalite("DIR21a", tableauDeBord.positionRouesAvant.tempsHaut.valeur, 65535 - 4040);

    return testsEnErreur;
}

void ignore_les_commandes_i2c_si_mode_telecommande() {
    initaliseEvenements();
    busOuTelecommande = MODE_TELECOMMANDE;
    
    receptionBus(0, 10);
    verifieEgalite("DIR_IGI2C0", (int) defileEvenement(), 0);
    
    receptionBus(1, 10);
    verifieEgalite("DIR_IGI2C1", (int) defileEvenement(), 0);
    
    receptionBus(2, 10);
    verifieEgalite("DIR_IGI2C2", (int) defileEvenement(), 0);
    
    receptionBus(3, 10);
    verifieEgalite("DIR_IGI2C3", (int) defileEvenement(), 0);
}

void transmet_les_commandes_i2c() {
    initaliseEvenements();
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    EvenementEtValeur *evenementEtValeur;
    
    receptionBus(0, 100);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_ACI2C0", evenementEtValeur->evenement, LECTURE_RC_AVANT_ARRIERE);
    verifieEgalite("DIR_ACI2C1", evenementEtValeur->valeur, 100);
    
    receptionBus(1, 110);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_ACI2C2", evenementEtValeur->evenement, LECTURE_RC_GAUCHE_DROITE);
    verifieEgalite("DIR_ACI2C3", evenementEtValeur->valeur, 110);
}

void transmet_les_commandes_de_la_telecommande() {
    initaliseEvenements();
    busOuTelecommande = MODE_TELECOMMANDE;
    EvenementEtValeur *evenementEtValeur;
        
    receptionTelecommandeAvantArriere(21);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_T0", evenementEtValeur->evenement, LECTURE_RC_AVANT_ARRIERE);
    verifieEgalite("DIR_T1", evenementEtValeur->valeur, 21);

    receptionTelecommandeGaucheDroite(22);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_T2", evenementEtValeur->evenement, LECTURE_RC_GAUCHE_DROITE);
    verifieEgalite("DIR_T3", evenementEtValeur->valeur, 22);
    
}

void expose_les_commandes_de_la_telecommande_a_i2c() {
    busOuTelecommande = MODE_TELECOMMANDE;
        
    receptionTelecommandeAvantArriere(121);
    receptionTelecommandeGaucheDroite(221);
    verifieEgalite("DIR_I2C0", i2cValeursExposees[0], 121);
    verifieEgalite("DIR_I2C1", i2cValeursExposees[1], 221);
}

void passe_en_mode_telecommande_si_le_canal_1_est_pas_neutre() {
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeAvantArriere(127);
    verifieEgalite("DIR_C1N0", busOuTelecommande, MODE_BUS_DE_COMMANDES);
    
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeAvantArriere(127 + SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C1N1", busOuTelecommande, MODE_TELECOMMANDE);

    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeAvantArriere(127 - SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C1N2", busOuTelecommande, MODE_TELECOMMANDE);
}

void passe_en_mode_telecommande_si_le_canal_2_est_pas_neutre() {
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeGaucheDroite(127);
    verifieEgalite("DIR_C2N0", busOuTelecommande, MODE_BUS_DE_COMMANDES);
    
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeGaucheDroite(127 + SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C2N1", busOuTelecommande, MODE_TELECOMMANDE);

    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeGaucheDroite(127 - SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C2N2", busOuTelecommande, MODE_TELECOMMANDE);
}

void passe_en_mode_bus_si_la_telecommande_est_longtemps_neutre() {
    int n;
    EvenementEtValeur ev = {BASE_DE_TEMPS, 0};


    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    for (n = 0; n < TEMPS_INACTIVITE_TELECOMMANDE; n++) {
        DIRECTION_machine(&ev);
    }
    verifieEgalite("DIR_N1", busOuTelecommande, MODE_BUS_DE_COMMANDES);
}

/**
 * Tests unitaires pour le positionnement des roues de direction.
 */
void test_direction() {
    calcule_pwm_servo_roues_avant();
    ignore_les_commandes_i2c_si_mode_telecommande();
    transmet_les_commandes_i2c();
    transmet_les_commandes_de_la_telecommande();
    expose_les_commandes_de_la_telecommande_a_i2c();
    passe_en_mode_telecommande_si_le_canal_1_est_pas_neutre();
    passe_en_mode_telecommande_si_le_canal_2_est_pas_neutre();
    passe_en_mode_bus_si_la_telecommande_est_longtemps_neutre();
}
#endif
