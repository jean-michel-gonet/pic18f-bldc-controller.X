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
void initialiseUART1()
{
    // Pour que la EUSART marche correctement il
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

/** Nombre de tests en erreur depuis l'initialisation des tests. */
static int testsEnErreur = 0;

/** Nombre de tests en succès depuis l'initialisation des tests. */
static int testsSucces = 0;

/** Initialise les tests.*/
void initialiseTests() {
    initialiseUART1();
    testsEnErreur = 0;
    printf("\r\nLancement des tests...\r\n");
}

/**
 * Vérifie si <param>value</param> et <param>expectedValue</param> sont
 * identiques. Si elles ne le sont pas, affiche le test en erreur.
 * @param valeurObtenue Valeur obtenue.
 * @param valeurAttendue Valeur attendue.
 * @param testId Identifiant du test.
 */
unsigned char verifieEgalite(const char *testId, int valeurObtenue, int valeurAttendue) {

    if (valeurObtenue != valeurAttendue) {
        printf("Test %s: attendu [%d], mais [%d]\r\n",
                testId, valeurAttendue, valeurObtenue);
        testsEnErreur ++;
        return 1;
    } else {
        testsSucces ++;
    }
    return 0;
}

/**
 * Vérifie si <param>value</param> se trouve entre les deux
 * limites.
 * @param valeurObtenue Valeur obtenue.
 * @param min La valeur obtenue ne doit pas être inférieure à min.
 * @param max La valeur obtenue ne doit pas être supérieure a max.
 * @param testId Identifiant du test.
 */
unsigned char verifieIntervale(const char *testId, int valeurObtenue, int min, int max) {

    if ( (valeurObtenue < min) || (valeurObtenue > max) ) {
        printf("Test %s: attendu entre [%d] et [%d], mais [%d]\r\n",
                testId, min, max, valeurObtenue);
        testsEnErreur ++;
        return 1;
    } else {
        testsSucces ++;
    }
    return 0;
}

/**
 * Vérifie si <param>value</param> zéro.
 * @param valeurObtenue Valeur obtenue.
 * @param testId Identifiant du test.
 */
unsigned char verifieNonZero(const char *testId, char valeurObtenue) {
    if (valeurObtenue == 0) {
        printf("Test %s: attendu [0], mais [%d]\r\n",
                testId, valeurObtenue);
        testsEnErreur ++;
        return 1;
    } else {
        testsSucces ++;
    }
    return 0;
}
/**
 * Finalise les tests.
 */
void finaliseTests() {
    printf("%d tests en succes\r\n", testsSucces);
    printf("%d tests en erreur\r\n", testsEnErreur);
}

#endif