#include "domaine.h"
#include "test.h"

/**
 * Soustrait deux magnitudes en tenant compte de leur direction.
 * @param a Magnitude A
 * @param b Magnitude B
 * @return A - B
 */
int compareAetB(MagnitudeEtDirection *a, 
                MagnitudeEtDirection *b) {
    int resultat;
    switch (a->direction) {
        case AVANT:
            resultat = a->magnitude;
            break;
        case ARRIERE:
            resultat = -a->magnitude;
            break;
        default:
            return 0;
    }
    switch (b->direction) {
        case ARRIERE:
            return resultat + b->magnitude;
        case AVANT:
        default:
            return resultat - b->magnitude;
    }    
}

/**
 * Réalise l'opération A = A - B
 * @param a Magnitude A
 * @param b Magnitude B
 * @return TRUE si la magnitude A a changé de signe pendant l'opération
 */
unsigned char opereAmoinsB(MagnitudeEtDirection *a, 
                           MagnitudeEtDirection *b) {
    switch (a->direction) {
        case AVANT:
            switch (b->direction) {
                case AVANT:
                    if (a->magnitude < b->magnitude) {
                        a->magnitude = b->magnitude - a->magnitude;
                        a->direction = ARRIERE;
                        return TRUE;
                    } else {
                        a->magnitude -= b->magnitude;
                        return FALSE;
                    }
                    
                case ARRIERE:
                    a->magnitude += b->magnitude;
                    return FALSE;
                    
                default:
                    return FALSE;
            }
        case ARRIERE:
            switch (b->direction) {
                case AVANT:
                    a->magnitude += b->magnitude;
                    return FALSE;
                    
                case ARRIERE:
                    if (a->magnitude < b->magnitude) {
                        a->magnitude = b->magnitude - a->magnitude;
                        a->direction = AVANT;
                        return TRUE;
                    } else {
                        a->magnitude -= b->magnitude;
                        return FALSE;
                    }
                    
                default:
                    return FALSE;
            }

        default:
            return FALSE;
    }
}

/**
 * Réalise l'opération A = A + B
 * @param a Magnitude A
 * @param b Magnitude B
 * @return TRUE si la magnitude A a changé de signe pendant l'opération
 */
unsigned char opereAplusB(MagnitudeEtDirection *a, 
                          MagnitudeEtDirection *b) {
    switch (a->direction) {
        case AVANT:
            switch (b->direction) {
                case AVANT:
                    a->magnitude += b->magnitude;
                    return FALSE;

                case ARRIERE:
                    if (a->magnitude < b->magnitude) {
                        a->magnitude = b->magnitude - a->magnitude;
                        a->direction = ARRIERE;
                        return TRUE;
                    } else {
                        a->magnitude -= b->magnitude;
                        return FALSE;
                    }
                    
                default:
                    return FALSE;
            }
        case ARRIERE:
            switch (b->direction) {
                case ARRIERE:
                    a->magnitude += b->magnitude;
                    return FALSE;
                    
                case AVANT:
                    if (a->magnitude < b->magnitude) {
                        a->magnitude = b->magnitude - a->magnitude;
                        a->direction = AVANT;
                        return TRUE;
                    } else {
                        a->magnitude -= b->magnitude;
                        return FALSE;
                    }
                    
                default:
                    return FALSE;
            }

        default:
            return FALSE;
    }
}

#ifdef TEST

void test_compare_A_et_B() {
    MagnitudeEtDirection a,b;

    // Signes opposés:
    a.direction = AVANT;
    b.direction = ARRIERE;
    
    a.magnitude = 100;
    b.magnitude = 50;
    verifieEgalite("SO01p", compareAetB(&a, &b),  150);
    verifieEgalite("SO01n", compareAetB(&b, &a), -150);
    
    a.magnitude = 50;
    b.magnitude = 100;
    verifieEgalite("SO02p", compareAetB(&a, &b),  150);
    verifieEgalite("SO02n", compareAetB(&b, &a), -150);

    // Signes identiques:
    a.direction = ARRIERE;
    b.direction = ARRIERE;
    
    a.magnitude = 100;
    b.magnitude = 50;
    verifieEgalite("SO03p", compareAetB(&a, &b), -50);
    verifieEgalite("SO03n", compareAetB(&b, &a),  50);
}

void test_opere_A_moins_B() {
    MagnitudeEtDirection a,b;
    
    a.direction = AVANT;
    a.magnitude = 100;
    b.direction = AVANT;
    b.magnitude = 50;
    verifieEgalite("OM00", opereAmoinsB(&a, &b), FALSE);
    verifieEgalite("OM01", a.direction, AVANT);
    verifieEgalite("OM02", a.magnitude, 50);
    
    a.direction = AVANT;
    a.magnitude = 30;
    b.direction = AVANT;
    b.magnitude = 70;
    verifieEgalite("OM10", opereAmoinsB(&a, &b), TRUE);
    verifieEgalite("OM11", a.direction, ARRIERE);
    verifieEgalite("OM12", a.magnitude, 40);

    a.direction = AVANT;
    a.magnitude = 30;
    b.direction = ARRIERE;
    b.magnitude = 70;
    verifieEgalite("OM20", opereAmoinsB(&a, &b), FALSE);
    verifieEgalite("OM21", a.direction, AVANT);
    verifieEgalite("OM22", a.magnitude, 100);

    
    a.direction = ARRIERE;
    a.magnitude = 100;
    b.direction = ARRIERE;
    b.magnitude = 50;
    verifieEgalite("OM30", opereAmoinsB(&a, &b), FALSE);
    verifieEgalite("OM31", a.direction, ARRIERE);
    verifieEgalite("OM32", a.magnitude, 50);
    
    a.direction = ARRIERE;
    a.magnitude = 30;
    b.direction = ARRIERE;
    b.magnitude = 70;
    verifieEgalite("OM40", opereAmoinsB(&a, &b), TRUE);
    verifieEgalite("OM41", a.direction, AVANT);
    verifieEgalite("OM42", a.magnitude, 40);

    a.direction = ARRIERE;
    a.magnitude = 30;
    b.direction = AVANT;
    b.magnitude = 70;
    verifieEgalite("OM50", opereAmoinsB(&a, &b), FALSE);
    verifieEgalite("OM51", a.direction, ARRIERE);
    verifieEgalite("OM52", a.magnitude, 100);
}

void test_opere_A_plus_B() {
    MagnitudeEtDirection a,b;
    
    a.direction = AVANT;
    a.magnitude = 30;
    b.direction = AVANT;
    b.magnitude = 50;
    verifieEgalite("OP00", opereAplusB(&a, &b), FALSE);
    verifieEgalite("OP01", a.direction, AVANT);
    verifieEgalite("OP02", a.magnitude, 80);
    
    a.direction = AVANT;
    a.magnitude = 70;
    b.direction = ARRIERE;
    b.magnitude = 30;
    verifieEgalite("OP10", opereAplusB(&a, &b), FALSE);
    verifieEgalite("OP11", a.direction, AVANT);
    verifieEgalite("OP12", a.magnitude, 40);

    a.direction = AVANT;
    a.magnitude = 30;
    b.direction = ARRIERE;
    b.magnitude = 80;
    verifieEgalite("OP20", opereAplusB(&a, &b), TRUE);
    verifieEgalite("OP21", a.direction, ARRIERE);
    verifieEgalite("OP22", a.magnitude, 50);

    
    a.direction = ARRIERE;
    a.magnitude = 20;
    b.direction = ARRIERE;
    b.magnitude = 50;
    verifieEgalite("OP30", opereAplusB(&a, &b), FALSE);
    verifieEgalite("OP31", a.direction, ARRIERE);
    verifieEgalite("OP32", a.magnitude, 70);
    
    a.direction = ARRIERE;
    a.magnitude = 70;
    b.direction = AVANT;
    b.magnitude = 30;
    verifieEgalite("OP40", opereAplusB(&a, &b), FALSE);
    verifieEgalite("OP41", a.direction, ARRIERE);
    verifieEgalite("OP42", a.magnitude, 40);

    a.direction = ARRIERE;
    a.magnitude = 60;
    b.direction = AVANT;
    b.magnitude = 70;
    verifieEgalite("OP50", opereAplusB(&a, &b), TRUE);
    verifieEgalite("OP51", a.direction, AVANT);
    verifieEgalite("OP52", a.magnitude, 10);
}

/**
 * Tests unitaires pour le calcul de tension.
 * @return Nombre de tests en erreur.
 */
void test_domaine() {
    test_compare_A_et_B();
    test_opere_A_moins_B();
    test_opere_A_plus_B();
}

#endif