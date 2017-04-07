#include <xc.h>
#include "i2c.h"
#include "file.h"
#include "test.h"

File fileEmission;

/**
 * @return 255 / -1 si il reste des données à émettre.
 */
unsigned char i2cDonneesDisponiblesPourEmission() {
    if (fileEstVide(&fileEmission)) {
        return 0;
    }
    return 255;
}

/**
 * Rend le prochain caractère à émettre.
 * Appelez i2cCommandeCompletementEmise avant, pour éviter
 * de consommer la prochaine commande.
 * @return 
 */
unsigned char i2cRecupereCaracterePourEmission() {
    return fileDefile(&fileEmission);
}

typedef enum {
    I2C_MASTER_EMISSION_ADRESSE,
    I2C_MASTER_PREPARE_RECEPTION_DONNEE,
    I2C_MASTER_RECEPTION_DONNEE,
    I2C_MASTER_EMISSION_DONNEE,
    I2C_MASTER_EMISSION_STOP,
    I2C_MASTER_FIN_OPERATION,
            
} EtatMaitreI2C;

static EtatMaitreI2C etatMaitre = I2C_MASTER_EMISSION_ADRESSE;

/**
 * Prépare l'émission de la commande indiquée.
 * La fonction revient immédiatement, sans attendre que la commande
 * soit transmise.
 * @param adresse Adresse de l'esclave. Si le bit moins signifiant 
 * est 1, le maître lit sur l'esclave (lecture).
 * @param valeur Valeur associée. Dans une opération de lecture, cette
 * valeur n'a pas d'effet.
 */
void i2cPrepareCommandePourEmission(I2cAdresse adresse, unsigned char valeur) {
    fileEnfile(&fileEmission, adresse);
    fileEnfile(&fileEmission, valeur);
    if (etatMaitre == I2C_MASTER_EMISSION_ADRESSE) {
        SSP1CON2bits.SEN = 1;
    }
}

/**
 * Fonction de rappel de commandes par défaut.
 */
void faitRienDuTout(unsigned char adresse, unsigned char valeur) {
    // Rien à faire.
}

/** 
 * Adresse de la fonction à appeler pour compléter 
 * l'exécution d'une commande I2C.
 * Par défaut elle pointe sur une fonction qui ne fait rien.
 */
static I2cRappelCommande rappelCommande = faitRienDuTout;

/**
 * Établit la fonction à appeler pour compléter l'exécution 
 * d'une commande I2C.
 * Le maître appelle cette fonction pour terminer l'exécution 
 * d'une commande de lecture. L'esclave appelle cette fonction pour 
 * gérer l'exécution d'une commande d'écriture.
 * @param r La fonction à appeler.
 */
void i2cRappelCommande(I2cRappelCommande r) {
    rappelCommande = r;
}

void i2cMaitre() {
    static unsigned char adresse; // Adresse associée à la commande en cours.
    
    switch (etatMaitre) {
        case I2C_MASTER_EMISSION_ADRESSE:
            if (i2cDonneesDisponiblesPourEmission()) {
                adresse = i2cRecupereCaracterePourEmission();
                if (adresse & 1) {
                    etatMaitre = I2C_MASTER_PREPARE_RECEPTION_DONNEE;
                } else {
                    etatMaitre = I2C_MASTER_EMISSION_DONNEE;
                }
                SSP1BUF = adresse;
            }
            break;
            
        case I2C_MASTER_EMISSION_DONNEE:
            etatMaitre = I2C_MASTER_EMISSION_STOP;
            SSP1BUF = i2cRecupereCaracterePourEmission();
            break;

        case I2C_MASTER_PREPARE_RECEPTION_DONNEE:
            etatMaitre = I2C_MASTER_RECEPTION_DONNEE;
            i2cRecupereCaracterePourEmission();
            SSP1CON2bits.RCEN = 1;  // MMSP en réception.
            break;
            
        case I2C_MASTER_RECEPTION_DONNEE:
            etatMaitre = I2C_MASTER_EMISSION_STOP;
            // Le maître doit gérer la valeur rendue par l'esclave
            rappelCommande(adresse, SSP1BUF);
            SSP1CON2bits.ACKDT = 1; // NACK
            SSP1CON2bits.ACKEN = 1; // Transmet le NACK
            break;
            
        case I2C_MASTER_EMISSION_STOP:
            etatMaitre = I2C_MASTER_FIN_OPERATION;
            SSP1CON2bits.PEN = 1;
            break;
            
        case I2C_MASTER_FIN_OPERATION:
            etatMaitre = I2C_MASTER_EMISSION_ADRESSE;
            if (i2cDonneesDisponiblesPourEmission()) {
                SSP1CON2bits.SEN = 1;
            }
            break;
    }
}
/**
 * Le bit moins signifiant de SSPxBUF contient R/W.
 * Il faut donc décaler l'adresse de 1 bit vers la droite.
 * @param adresse Adresse reçue par le bus I2C.
 * @return Adresse locale.
 */
unsigned char convertitEnAdresseLocale(unsigned char adresse) {
    adresse >>= 1;
    adresse &= I2C_MASQUE_ADRESSES_LOCALES;
    return adresse;
}

/**
 * L'esclave rendra la valeur indiquée à prochaine lecture de 
 * l'adresse indiquée sur le bus I2C.
 * @param adresse Adresse locale, entre 0 et 4 (l'adresse locale 
 * est constituée des 2 bits moins signifiants de l'adresse 
 * demandée par le maître).
 * @param valeur La valeur.
 */
void i2cExposeValeur(unsigned char adresse, unsigned char valeur) {
    i2cValeursExposees[adresse & I2C_MASQUE_ADRESSES_LOCALES] = valeur;
}

/**
 * Automate esclave I2C.
 * @param valeursLecture Liste de valeurs à rendre en cas d'opération
 * de lecture.
 */
void i2cEsclave() {
    static unsigned char adresse;
    
    // Machine à état extraite de Microchip AN00734b - Appendice B
    if (SSP1STATbits.S) {
        if (SSP1STATbits.RW) {
            // État 4 - Opération de lecture, dernier octet transmis est une donnée:
            // Jamais utilisé si les commandes ont un seul octet associé.
            if (SSP1STATbits.DA) {
                SSP1BUF = i2cValeursExposees[adresse];
                SSP1CON1bits.CKP = 1;
            } 
            // État 3 - Opération de lecture, dernier octet reçu est une adresse:
            else {
                adresse = convertitEnAdresseLocale(SSP1BUF);
                SSP1BUF = i2cValeursExposees[adresse];
                SSP1CON1bits.CKP = 1;
                // Sur les PIC18 plus récents, BF s'allume en État 3.
                // Il doit être lu et désactivé.
                if (SSP1STATbits.BF) {
                    SSP1STATbits.BF = 0;
                }
            }
        } else {
            // État 2 - Opération d'écriture, dernier octet reçu est une donnée:
            if (SSP1STATbits.DA) {
                // L'esclave doit traiter la donnée reçue:
                rappelCommande(adresse, SSP1BUF);
            }
            // État 1 - Opération d'écriture, dernier octet reçu est une adresse:
            else {
                adresse = convertitEnAdresseLocale(SSP1BUF);
                if (SSP1CON1bits.SSPOV) {
                    SSP1CON1bits.SSPOV = 0;
                }
            }
        }
    }
    PIR1bits.SSP1IF = 0;
}

/**
 * Réinitialise la machine i2c.
 */
void i2cReinitialise() {
    etatMaitre = I2C_MASTER_EMISSION_ADRESSE;
    fileReinitialise(&fileEmission);
}