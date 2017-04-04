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
void initialiseUART1();

/** Initialise les tests. */
void initialiseTests();

/**
 * Vérifie si <param>value</param et <param>expectedValue</param> sont
 * identiques. Si elles ne le sont pas, affiche le test en erreur.
 * @param valeurObtenue Valeur obtenue.
 * @param valeurAttendue Valeur attendue.
 * @param testId Identifiant du test.
 */
unsigned char verifieEgalite(const char *testId, int valeurObtenue, int valeurAttendue);

/**
 * Vérifie si <param>value</param> se trouve entre les deux
 * limites.
 * @param valeurObtenue Valeur obtenue.
 * @param min La valeur obtenue ne doit pas être inférieure à min.
 * @param max La valeur obtenue ne doit pas être supérieure à max.
 * @param testId Identifiant du test.
 */
unsigned char verifieIntervale(const char *testId, int valeurObtenue, int min, int max);

/**
 * Vérifie si <param>value</param n'est pas zéro.
 * @param valueObtenue Valeur obtenue.
 * @param testId Identifiant du test.
 */
unsigned char verifieNonZero(const char *testId, char valeurObtenue);

/** Finalise les tests. */
void finaliseTests();

#endif
#endif	/* TEST_H */