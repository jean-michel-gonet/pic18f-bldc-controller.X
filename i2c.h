#ifndef I2C__H
#define I2C__H

#define          I2C_ADRESSE_DE_BASE 0b00100000
#define  I2C_MASQUE_ADRESSES_LOCALES 0b00000111
#define I2C_MASQUE_ADRESSES_ESCLAVES 0b11110000

typedef enum {
    ECRITURE_I2C_VITESSE                  = 0,
    ECRITURE_I2C_DIRECTION                = 1,
    ECRITURE_I2C_MANOEUVRE                = 2,

    LECTURE_I2C_VITESSE_RC                = 0, // 0x10
    LECTURE_I2C_RC_GAUCHE_DROITE          = 1, // 0x11
    LECTURE_I2C_INACTIVITE_TELECOMMANDE   = 2, // 0x12
    LECTURE_I2C_LECTURE_ERREUR_I          = 3, // 0x13
    LECTURE_I2C_DERNIERE_MANOEUVRE_RECUE  = 4, // 0x14
    LECTURE_I2C_NOMBRE_DE_MANOEUVRES      = 5, // 0x15
    LECTURE_I2C_N                         = 6  // 0x16
            
} I2cAdresse;

typedef struct {
    I2cAdresse adresse;
    unsigned char valeur;
} I2cCommande;

/** Liste des valeurs exposées par l'esclave I2C. */
unsigned char i2cValeursExposees[I2C_MASQUE_ADRESSES_LOCALES + 1];

typedef void (*I2cRappelCommande)(unsigned char, unsigned char);
void i2cRappelCommande(I2cRappelCommande r);
void i2cExposeValeur(unsigned char adresse, unsigned char valeur);
void i2cPrepareCommandePourEmission(I2cAdresse adresse, unsigned char valeur);
unsigned char i2cDonneesDisponiblesPourEmission();
unsigned char i2cRecupereCaracterePourEmission();

void i2cMaitre();
void i2cEsclave();

void i2cReinitialise();

#ifdef TEST
void testI2c();
#endif

#endif