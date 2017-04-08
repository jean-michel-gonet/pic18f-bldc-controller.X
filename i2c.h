#ifndef I2C__H
#define I2C__H

#define  I2C_MASQUE_ADRESSES_LOCALES 0b00000011
#define I2C_MASQUE_ADRESSES_ESCLAVES 0b11111000

typedef enum {
    ECRITURE_VITESSE          = 0b00100000,
    ECRITURE_DIRECTION        = 0b00100001,
    ECRITURE_MANOEUVRE        = 0b00100010,

    LECTURE_VITESSE_RC        = 0b00100000,
    LECTURE_DIRECTION_RC      = 0b00100001,
    LECTURE_ERREUR_POSTION    = 0b00100010,
    LECTURE_NOMBRE_MANOEUVRES = 0b00100010
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