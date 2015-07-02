#ifdef TEST

#include <htc.h>
#include <stdio.h>

/**
 * Fonction qui transmet un caractère à la EUSART.
 * Il s'agit de l'implémentation d'une fonction système qui est
 * appelée par <code>printf</code>.
 * Si un terminal est connecté aux sorties RX / TX, il affichera du texte.
 * @param data Le code ascii du caractère à afficher.
 */
void putch(char data) {
    while( ! TX1IF);
    TXREG1 = data;
}

/**
 * Configuration de la EUSART comme sortie asynchrone à 1200 bauds.
 * On assume que le PIC18 fonctionne à Fosc = 1MHz.
 */
void EUSART_Initialize()
{
    // Pour que la EUSART marche correctement sur Proteus, il
    // faut désactiver les convertisseurs A/D.
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;

    // Pour une fréquence de 16MHz, ceci donne 19200 bauds:
    SPBRG1 = 12;
    SPBRGH1 = 0;

    // Configure RC6 et RC7 comme entrées digitales, pour que
    // la EUSART puisse en prendre le contrôle:
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;

    // Configure la EUSART:
    RCSTA1bits.SPEN = 1;  // Active la EUSART.
    TXSTA1bits.SYNC = 0;  // Mode asynchrone.
    TXSTA1bits.TXEN = 1;  // Active l'émetteur.
}


/**
 * Vérifie si <param>value</param et <param>expectedValue</param> sont
 * identiques. Si elles ne le sont pas, affiche le test en erreur.
 * @param value Valeur obtenue.
 * @param expectedValue Valeur attendue.
 * @param testId Identifiant du test.
 */
unsigned char assertEqualsInt(int value, int expectedValue, const char *testId) {

    if (value != expectedValue) {
        printf("Test %s a echoue: attendu [%d], mais [%d]\r\n",
                testId, expectedValue, value);
        return 1;
    }
    return 0;
}

/**
 * Vérifie si <param>value</param> se trouve entre les deux
 * limites.
 * @param value Valeur obtenue.
 * @param min La valeur obtenue ne doit pas être inférieure à min.
 * @param max La valeur obtenue ne doit pas être supérieure à max.
 * @param testId Identifiant du test.
 */
unsigned char assertMinMaxInt(int value, int min, int max, const char *testId) {

    if ( (value < min) || (value > max) ) {
        printf("Test %s a echoue: attendu entre [%d] et [%d], mais [%d]\r\n",
                testId, min, max, value);
        return 1;
    }
    return 0;
}


/**
 * Vérifie si <param>value</param et <param>expectedValue</param> sont
 * identiques. Si elles ne le sont pas, affiche le test en erreur.
 * @param value Valeur obtenue.
 * @param expectedValue Valeur attendue.
 * @param testId Identifiant du test.
 */
unsigned char assertEqualsChar(char value, char expectedValue, const char *testId) {
    if (value != expectedValue) {
        printf("Test %s a echoue: attendu [%d], mais [%d]\r\n",
                testId, expectedValue, value);
        return 1;
    }
    return 0;
}

/**
 * Vérifie si <param>value</param n'est pas zéro.
 * @param value Valeur obtenue.
 * @param testId Identifiant du test.
 */
unsigned char assertNotZeroChar(char value, const char *testId) {
    if (value == 0) {
        printf("Test %s a echoue: attendu [0], mais [%d]\r\n",
                testId, value);
        return 1;
    }
    return 0;
}

#endif