/* 
 * File:   test.h
 * Author: jmgonet
 *
 * Created on 31. janvier 2013, 06:56
 */

#ifndef TEST_H
#define	TEST_H

#ifdef TEST

/**
 * Configuration de la EUSART comme sortie asynchrone à 1200 bauds.
 * On assume que le PIC18 fonctionne à Fosc = 1MHz.
 */
void EUSART_Initialize();

/**
 * Affiche le code du test qui a échoué.
 * @param failedTestId L'identifiant du test qui a échoué.
 */
void displayError(unsigned int failedTestId);

/**
 * Vérifie si <param>value</param et <param>expectedValue</param> sont
 * identiques. Si elles ne le sont pas, affiche le test en erreur.
 * @param value Valeur obtenue.
 * @param expectedValue Valeur attendue.
 * @param testId Identifiant du test.
 */
unsigned char assertEqualsInt(int value,
        int expectedValue, const char *testId);

/**
 * Vérifie si <param>value</param> se trouve entre les deux
 * limites.
 * @param value Valeur obtenue.
 * @param min La valeur obtenue ne doit pas être inférieure à min.
 * @param max La valeur obtenue ne doit pas être supérieure à max.
 * @param testId Identifiant du test.
 */
unsigned char assertMinMaxInt(int value, int min, int max, const char *testId);

/**
 * Vérifie si <param>value</param et <param>expectedValue</param> sont
 * identiques. Si elles ne le sont pas, affiche le test en erreur.
 * @param value Valeur obtenue.
 * @param expectedValue Valeur attendue.
 * @param testId Identifiant du test.
 */
unsigned char assertEqualsChar(char value, 
        char expectedValue, const char *testId);

/**
 * Vérifie si <param>value</param n'est pas zéro.
 * @param value Valeur obtenue.
 * @param testId Identifiant du test.
 */
unsigned char assertNotZeroChar(char value, const char *testId);

#endif
#endif	/* TEST_H */