#include "test.h"
#include "tableauDeBord.h"
#include "direction.h"
#include "evenements.h"
#include "i2c.h"
#include "file.h"

/** 
 * Distance du neutre en deçà de la quelle on considère que la télécommande
 * est centrée.
 */
#define SEUIL_NEUTRALITE_TELECOMMANDE 10

/** 
 * Comptes de la base de temps jusqu'à considérer 
 * que la télécommande n'est plus active (environ 5 secondes). 
 */
#define TEMPS_INACTIVITE_TELECOMMANDE 35

/**
 * Décrit une manoeuvre.
 */
typedef struct {
    /** Distance à parcourir. */
    unsigned char distance;
    
    /** Orientation des roues. */
    unsigned char orientationRoues;
} Manoeuvre;

/**
 * Liste des manoeuvres.
 */
const Manoeuvre const manoeuvres[] = {
//   Distance  //  Orientation des roues
    {NEUTRE +  95, NEUTRE +  0},     // Avance un peu.
    {NEUTRE +  95, NEUTRE + 90},     // Quart de tour avant gauche
    {NEUTRE +  95, NEUTRE - 90},     // Quart de tour avant droit
    {NEUTRE -  95, NEUTRE +  0},     // Recule un peu.
    {NEUTRE -  95, NEUTRE + 90},     // Quart de tour arrière gauche.
    {NEUTRE -  95, NEUTRE - 90}      // Quart de tour arrière droit
};

/**
 * Mode de direction.
 */
typedef enum {
    /** C'est la télécommande qui commande.*/
    MODE_TELECOMMANDE,
            
    /** C'est le bus qui commande.*/
    MODE_BUS_DE_COMMANDES
} BusOuTelecommande;

/** Indique qui commande.*/
BusOuTelecommande busOuTelecommande = MODE_TELECOMMANDE;

/** 
 * Nombre d'impulsions de la base de temps jusqu'à considérer
 * que la télécommande n'est plus active.
 */
unsigned char tempsInactiviteTelecommande = TEMPS_INACTIVITE_TELECOMMANDE;

/** 
 * État des manoeuvres. 
 */
typedef enum {
    MANOEUVRE_EN_COURS,
    PAS_DE_MANOEUVRE
} EtatManoeuvre;

/** Indique si il y a une manoeuvre en cours. */
EtatManoeuvre etatManoeuvre = PAS_DE_MANOEUVRE;

/**
 * Indique le nombre de manoeuvres à exécuter (y compris la manoeuvre en cours).
 */
unsigned char nombreDeManoeuvresAExecuter = 0;

/**
 * File de manoeuvres.
 */
File fileManoeuvres;

/**
 * Vide la file des manoeuvres.
 */
void reinitialiseManoeuvres() {
    fileReinitialise(&fileManoeuvres);
    nombreDeManoeuvresAExecuter = 0;
    etatManoeuvre = PAS_DE_MANOEUVRE;
    i2cExposeValeur(LECTURE_I2C_NOMBRE_DE_MANOEUVRES, 0);
}

/**
 * Réinitialise le module de direction.
 * Y compris la file des manoeuvres.
 */
void initialiseDirection() {
    reinitialiseManoeuvres();
    busOuTelecommande = MODE_TELECOMMANDE;
    tempsInactiviteTelecommande = TEMPS_INACTIVITE_TELECOMMANDE;
}

/**
 * Ajoute une nouvelle manoeuvre à la file.
 * @param numeroDeManoeuvre Le numéro de manoeuvre.
 */
void enfileManoeuvre(unsigned char numeroDeManoeuvre) {
    if (!fileEstPleine(&fileManoeuvres)) {
        i2cExposeValeur(LECTURE_I2C_DERNIERE_MANOEUVRE_RECUE, numeroDeManoeuvre);
        fileEnfile(&fileManoeuvres, numeroDeManoeuvre);            
        nombreDeManoeuvresAExecuter ++;
        i2cExposeValeur(LECTURE_I2C_NOMBRE_DE_MANOEUVRES, nombreDeManoeuvresAExecuter);
    }
}

/**
 * Exécute la manoeuvre indiquée en plaçant les événements nécessaires.
 * @param numeroDeManoeuvre Le numéro de manoeuvre.
 */
void executeManoeuvre(unsigned char numeroDeManoeuvre) {
    Manoeuvre const *manoeuvre;
 
    manoeuvre = &(manoeuvres[numeroDeManoeuvre]);
    enfileMessageInterne(DEPLACEMENT_DEMANDE, manoeuvre->distance);
    enfileMessageInterne(LECTURE_RC_GAUCHE_DROITE, manoeuvre->orientationRoues);
}

/**
 * Si il y en a disponibles, défile une manoeuvre et l'exécute.
 */
void defileManoeuvre() {
    unsigned char numeroDeManoeuvre;
    if (!fileEstVide(&fileManoeuvres)) {
        numeroDeManoeuvre = fileDefile(&fileManoeuvres);
        executeManoeuvre(numeroDeManoeuvre);
        nombreDeManoeuvresAExecuter --;
    } else {
        nombreDeManoeuvresAExecuter = 0;
    }
    i2cExposeValeur(LECTURE_I2C_NOMBRE_DE_MANOEUVRES, nombreDeManoeuvresAExecuter);
}

/**
 * Reçoit les commandes en provenance du bus I2C.
 * Ignore les commandes si c'est la télécommande qui a le contrôle.
 * Les commandes de vitesse et direction vident la file de manoeuvres
 * et annulent la manoeuvre en cours.
 * @param adresse Adresse associée à la commande.
 * @param valeur Valeur associée à la commande.
 */
void receptionBus(unsigned char adresse, unsigned char valeur) {
    if (busOuTelecommande == MODE_BUS_DE_COMMANDES) {
        switch(adresse) {
            
            case ECRITURE_I2C_VITESSE:
                reinitialiseManoeuvres();
                enfileEvenement(VITESSE_DEMANDEE, valeur);
                break;
                
            case ECRITURE_I2C_DIRECTION:
                reinitialiseManoeuvres();
                enfileEvenement(LECTURE_RC_GAUCHE_DROITE, valeur);    
                break;
                
            case ECRITURE_I2C_MANOEUVRE:
                enfileManoeuvre(valeur);
                break;
                
            default:
                break;
        }
    }
}

/**
 * Indique si la valeur lue de la télécommande est considérée comme neutre.
 * @param valeur Valeur lue de la télécommande
 * @return -1/255 si la valeur n'est pas neutre.
 */
char valeurTelecommandeEstPasNeutre(unsigned char valeur) {
    if ((valeur >= NEUTRE + SEUIL_NEUTRALITE_TELECOMMANDE) ||
            (valeur <= NEUTRE - SEUIL_NEUTRALITE_TELECOMMANDE)) {
        return -1;
    } else {
        return 0;
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
            if (valeurTelecommandeEstPasNeutre(valeur)) {
                reinitialiseManoeuvres();
                busOuTelecommande = MODE_TELECOMMANDE;
                enfileEvenement(evenement, valeur);    
            }
            break;
        case MODE_TELECOMMANDE:
            if (valeurTelecommandeEstPasNeutre(valeur)) {
                tempsInactiviteTelecommande = TEMPS_INACTIVITE_TELECOMMANDE;
            }
            enfileEvenement(evenement, valeur);    
            break;
    }    
}

/**
 * Reçoit les lectures du canal 1 (avant / arrière) de la télécommande.
 * @param valeur Entre 0 et 255. 128 pour tout droit.
 */
void receptionTelecommandeAvantArriere(unsigned char valeur) {
    i2cExposeValeur(LECTURE_I2C_VITESSE_RC, valeur);
    receptionTelecommande(VITESSE_DEMANDEE, valeur);
}

/**
 * Reçoit les lectures du canal 2 (gauche / droite) de la télécommande.
 * @param valeur Entre 0 et 255. 128 pour repos.
 */
void receptionTelecommandeGaucheDroite(unsigned char valeur) {
    i2cExposeValeur(LECTURE_I2C_RC_GAUCHE_DROITE, valeur);
    receptionTelecommande(LECTURE_RC_GAUCHE_DROITE, valeur);    
}

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
 * Machine à états pour gérer la direction de la voiture.
 * @param ev Événement à traiter.
 */
void DIRECTION_machine(EvenementEtValeur *ev) {
    switch (ev->evenement) {
        case BASE_DE_TEMPS:
            if (tempsInactiviteTelecommande > 0) {
                tempsInactiviteTelecommande--;
                i2cExposeValeur(LECTURE_I2C_INACTIVITE_TELECOMMANDE, tempsInactiviteTelecommande);
                if (tempsInactiviteTelecommande == 0) {
                    busOuTelecommande = MODE_BUS_DE_COMMANDES;
                }
            }
            break;
            
        case LECTURE_RC_GAUCHE_DROITE:
            calculePwmServoRouesAvant(ev->valeur);
            break;
            
        case DEPLACEMENT_ATTEINT:
            defileManoeuvre();
            break;
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
    initialiseEvenements();
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
    initialiseEvenements();
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    EvenementEtValeur *evenementEtValeur;
    
    receptionBus(0, 100);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_ACI2C0", evenementEtValeur->evenement, VITESSE_DEMANDEE);
    verifieEgalite("DIR_ACI2C1", evenementEtValeur->valeur, 100);
    
    receptionBus(1, 110);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_ACI2C2", evenementEtValeur->evenement, LECTURE_RC_GAUCHE_DROITE);
    verifieEgalite("DIR_ACI2C3", evenementEtValeur->valeur, 110);
}

void transmet_les_commandes_de_la_telecommande() {
    initialiseEvenements();
    busOuTelecommande = MODE_TELECOMMANDE;
    EvenementEtValeur *evenementEtValeur;
        
    receptionTelecommandeAvantArriere(21);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_T0", evenementEtValeur->evenement, VITESSE_DEMANDEE);
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
    receptionTelecommandeAvantArriere(NEUTRE);
    verifieEgalite("DIR_C1N0", busOuTelecommande, MODE_BUS_DE_COMMANDES);
    
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeAvantArriere(NEUTRE + SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C1N1", busOuTelecommande, MODE_TELECOMMANDE);

    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeAvantArriere(NEUTRE - SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C1N2", busOuTelecommande, MODE_TELECOMMANDE);
}

void passe_en_mode_telecommande_si_le_canal_2_est_pas_neutre() {
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeGaucheDroite(NEUTRE);
    verifieEgalite("DIR_C2N0", busOuTelecommande, MODE_BUS_DE_COMMANDES);
    
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeGaucheDroite(NEUTRE + SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C2N1", busOuTelecommande, MODE_TELECOMMANDE);

    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    receptionTelecommandeGaucheDroite(NEUTRE - SEUIL_NEUTRALITE_TELECOMMANDE);
    verifieEgalite("DIR_C2N2", busOuTelecommande, MODE_TELECOMMANDE);
}

void passe_en_mode_bus_si_la_telecommande_est_longtemps_neutre() {
    int n;
    EvenementEtValeur ev = {BASE_DE_TEMPS, 0};

    busOuTelecommande = MODE_TELECOMMANDE;
    for (n = 0; n < TEMPS_INACTIVITE_TELECOMMANDE; n++) {
        receptionTelecommandeGaucheDroite(NEUTRE - SEUIL_NEUTRALITE_TELECOMMANDE + 1);
        receptionTelecommandeAvantArriere(NEUTRE + SEUIL_NEUTRALITE_TELECOMMANDE - 1);
        DIRECTION_machine(&ev);
    }
    verifieEgalite("DIR_N1", busOuTelecommande, MODE_BUS_DE_COMMANDES);
}

void execute_immediatement_la_premiere_manoeuvre() {
    initialiseMessagesInternes();
    initialiseDirection();
    EvenementEtValeur deplacementAtteint = {DEPLACEMENT_ATTEINT, 0};
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    EvenementEtValeur *evenementEtValeur;
    
    receptionBus(ECRITURE_I2C_MANOEUVRE, 1);
    receptionBus(ECRITURE_I2C_MANOEUVRE, 2);
    
    DIRECTION_machine(&deplacementAtteint);

    evenementEtValeur = defileMessageInterne();
    verifieEgalite("DIR_MAP0", evenementEtValeur->evenement, DEPLACEMENT_DEMANDE);
    verifieEgalite("DIR_MAP1", evenementEtValeur->valeur, manoeuvres[1].distance);
    
    evenementEtValeur = defileMessageInterne();
    verifieEgalite("DIR_MAP2", evenementEtValeur->evenement, LECTURE_RC_GAUCHE_DROITE);
    verifieEgalite("DIR_MAP3", evenementEtValeur->valeur, manoeuvres[1].orientationRoues);

    verifieEgalite("DIR_MAP4", (int) defileMessageInterne(), 0);
}

void execute_la_suivante_manoeuvre_apres_avoir_complete_la_premiere() {
    initialiseMessagesInternes();
    initialiseDirection();
    EvenementEtValeur deplacementAtteint = {DEPLACEMENT_ATTEINT, 0};
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    EvenementEtValeur *evenementEtValeur;
    
    receptionBus(2, 1);
    receptionBus(2, 2);
    
    DIRECTION_machine(&deplacementAtteint);
    defileMessageInterne();
    defileMessageInterne();
    verifieEgalite("DIR_MASU00", (int) defileMessageInterne(), 0);
    verifieEgalite("DIR_MASU01", nombreDeManoeuvresAExecuter, 1);

    DIRECTION_machine(&deplacementAtteint);
    evenementEtValeur = defileMessageInterne();
    verifieEgalite("DIR_MASU03", evenementEtValeur->evenement, DEPLACEMENT_DEMANDE);
    verifieEgalite("DIR_MASU04", evenementEtValeur->valeur, manoeuvres[2].distance);
    evenementEtValeur = defileMessageInterne();
    verifieEgalite("DIR_MASU05", evenementEtValeur->evenement, LECTURE_RC_GAUCHE_DROITE);
    verifieEgalite("DIR_MASU06", evenementEtValeur->valeur, manoeuvres[2].orientationRoues);

    defileManoeuvre();
    verifieEgalite("DIR_MASU07", nombreDeManoeuvresAExecuter, 0);    
}

void execute_un_arret_apres_avoir_complete_la_derniere_manoeuvre() {
    initialiseMessagesInternes();
    initialiseDirection();
    EvenementEtValeur deplacementAtteint = {DEPLACEMENT_ATTEINT, 0};
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    
    receptionBus(2, 1);
    receptionBus(2, 2);

    DIRECTION_machine(&deplacementAtteint);
    defileMessageInterne();
    defileMessageInterne();

    DIRECTION_machine(&deplacementAtteint);
    defileMessageInterne();
    defileMessageInterne();


    DIRECTION_machine(&deplacementAtteint);
    verifieEgalite("DIR_MAAR01", (int) defileMessageInterne(), 0);
}
void ignore_les_manoeuvres_si_la_file_deborde() {
    unsigned char n;
    reinitialiseManoeuvres();
    for(n = 0; n < FILE_TAILLE; n++) {
        receptionBus(2, 1);    
    }
    verifieEgalite("DIR_MAD01", nombreDeManoeuvresAExecuter, FILE_TAILLE);
    receptionBus(2, 1);    
    verifieEgalite("DIR_MAD02", nombreDeManoeuvresAExecuter, FILE_TAILLE);    
}

void reinitialise_les_manoeuvres_si_commande_de_vitesse() {
    reinitialiseManoeuvres();
    initialiseEvenements();
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    EvenementEtValeur *evenementEtValeur;

    receptionBus(2, 1);
    evenementEtValeur = defileEvenement();
    evenementEtValeur = defileEvenement();

    receptionBus(2, 1);
    receptionBus(2, 1);
    receptionBus(2, 1);
    
    receptionBus(0, 12);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_MARVD0", evenementEtValeur->evenement, VITESSE_DEMANDEE);
    verifieEgalite("DIR_MARVD1", evenementEtValeur->valeur, 12);
    verifieEgalite("DIR_MARVD2", nombreDeManoeuvresAExecuter, 0);       
    verifieEgalite("DIR_MARVD3", (int) defileEvenement(), 0);
}
void reinitialise_les_manoeuvres_si_commande_de_orientation_des_roues() {
    reinitialiseManoeuvres();
    initialiseEvenements();
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    EvenementEtValeur *evenementEtValeur;

    receptionBus(2, 1);
    evenementEtValeur = defileEvenement();
    evenementEtValeur = defileEvenement();

    receptionBus(2, 1);
    receptionBus(2, 1);
    receptionBus(2, 1);
    
    receptionBus(1, 12);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_MARDD0", evenementEtValeur->evenement, LECTURE_RC_GAUCHE_DROITE);
    verifieEgalite("DIR_MARDD1", evenementEtValeur->valeur, 12);
    verifieEgalite("DIR_MARDD2", nombreDeManoeuvresAExecuter, 0);       
    verifieEgalite("DIR_MARDD3", (int) defileEvenement(), 0);    
}

void reinitialise_les_manoeuvres_si_telecommande() {
    reinitialiseManoeuvres();
    initialiseEvenements();
    busOuTelecommande = MODE_BUS_DE_COMMANDES;
    EvenementEtValeur *evenementEtValeur;

    receptionBus(2, 1);
    evenementEtValeur = defileEvenement();
    evenementEtValeur = defileEvenement();

    receptionBus(2, 1);
    receptionBus(2, 1);
    receptionBus(2, 1);
    
    receptionTelecommandeAvantArriere(10);
    evenementEtValeur = defileEvenement();
    verifieEgalite("DIR_MART0", evenementEtValeur->evenement, VITESSE_DEMANDEE);
    verifieEgalite("DIR_MART1", evenementEtValeur->valeur, 10);
    verifieEgalite("DIR_MART2", nombreDeManoeuvresAExecuter, 0);       
    verifieEgalite("DIR_MART3", (int) defileEvenement(), 0);
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
    
    execute_immediatement_la_premiere_manoeuvre();
    execute_la_suivante_manoeuvre_apres_avoir_complete_la_premiere();
    execute_un_arret_apres_avoir_complete_la_derniere_manoeuvre();
    ignore_les_manoeuvres_si_la_file_deborde();
    reinitialise_les_manoeuvres_si_commande_de_vitesse();
    reinitialise_les_manoeuvres_si_commande_de_orientation_des_roues();
    reinitialise_les_manoeuvres_si_telecommande();
}
#endif
