#include "domaine.h"
#include "test.h"
#include "moteur.h"

/**
 * Table du sinus, précalculée, et stockée
 * en mémoire programme.
 */
#define XX 0
const unsigned char const sinus[65][19] = {
//    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
    { XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX,  XX },
    { XX,  XX,  14,  14,  15,  15,  16,  16,  16,  16,  16,  16,  16,  15,  15,  14,  14,  XX,  XX },
    { XX,  14,  15,  16,  17,  18,  19,  20,  20,  20,  20,  20,  19,  18,  17,  16,  15,  14,  XX },
    { XX,  14,  16,  18,  20,  21,  22,  23,  24,  24,  24,  23,  22,  21,  20,  18,  16,  14,  XX },
    { XX,  15,  18,  20,  22,  24,  26,  27,  27,  28,  27,  27,  26,  24,  22,  20,  18,  15,  XX },
    { XX,  16,  19,  22,  25,  27,  29,  30,  31,  31,  31,  30,  29,  27,  25,  22,  19,  16,  XX },
    { XX,  16,  20,  24,  27,  30,  32,  34,  35,  35,  35,  34,  32,  30,  27,  24,  20,  16,  XX },
    { XX,  17,  22,  26,  30,  33,  35,  37,  39,  39,  39,  37,  35,  33,  30,  26,  22,  17,  XX },
    { XX,  18,  23,  28,  32,  36,  39,  41,  42,  43,  42,  41,  39,  36,  32,  28,  23,  18,  XX },
    { XX,  18,  24,  30,  34,  39,  42,  44,  46,  47,  46,  44,  42,  39,  34,  30,  24,  18,  XX },
    { XX,  19,  25,  31,  37,  41,  45,  48,  50,  50,  50,  48,  45,  41,  37,  31,  25,  19,  XX },
    { XX,  20,  27,  33,  39,  44,  49,  52,  53,  54,  53,  52,  49,  44,  39,  33,  27,  20,  XX },
    { XX,  20,  28,  35,  42,  47,  52,  55,  57,  58,  57,  55,  52,  47,  42,  35,  28,  20,  XX },
    { XX,  21,  29,  37,  44,  50,  55,  59,  61,  62,  61,  59,  55,  50,  44,  37,  29,  21,  XX },
    { XX,  22,  31,  39,  47,  53,  58,  62,  65,  65,  65,  62,  58,  53,  47,  39,  31,  22,  XX },
    { XX,  22,  32,  41,  49,  56,  62,  66,  68,  69,  68,  66,  62,  56,  49,  41,  32,  22,  XX },
    { XX,  23,  33,  43,  51,  59,  65,  69,  72,  73,  72,  69,  65,  59,  51,  43,  33,  23,  XX },
    { XX,  24,  34,  45,  54,  62,  68,  73,  76,  77,  76,  73,  68,  62,  54,  45,  34,  24,  XX },
    { XX,  24,  36,  47,  56,  65,  71,  76,  80,  81,  80,  76,  71,  65,  56,  47,  36,  24,  XX },
    { XX,  25,  37,  48,  59,  68,  75,  80,  83,  84,  83,  80,  75,  68,  59,  48,  37,  25,  XX },
    { XX,  26,  38,  50,  61,  70,  78,  84,  87,  88,  87,  84,  78,  70,  61,  50,  38,  26,  XX },
    { XX,  26,  40,  52,  64,  73,  81,  87,  91,  92,  91,  87,  81,  73,  64,  52,  40,  26,  XX },
    { XX,  27,  41,  54,  66,  76,  85,  91,  94,  96,  94,  91,  85,  76,  66,  54,  41,  27,  XX },
    { XX,  28,  42,  56,  68,  79,  88,  94,  98,  99,  98,  94,  88,  79,  68,  56,  42,  28,  XX },
    { XX,  28,  44,  58,  71,  82,  91,  98, 102, 103, 102,  98,  91,  82,  71,  58,  44,  28,  XX },
    { XX,  29,  45,  60,  73,  85,  94, 101, 106, 107, 106, 101,  94,  85,  73,  60,  45,  29,  XX },
    { XX,  30,  46,  62,  76,  88,  98, 105, 109, 111, 109, 105,  98,  88,  76,  62,  46,  30,  XX },
    { XX,  30,  47,  64,  78,  91, 101, 108, 113, 115, 113, 108, 101,  91,  78,  64,  47,  30,  XX },
    { XX,  31,  49,  65,  81,  94, 104, 112, 117, 118, 117, 112, 104,  94,  81,  65,  49,  31,  XX },
    { XX,  32,  50,  67,  83,  96, 107, 116, 121, 122, 121, 116, 107,  96,  83,  67,  50,  32,  XX },
    { XX,  32,  51,  69,  85,  99, 111, 119, 124, 126, 124, 119, 111,  99,  85,  69,  51,  32,  XX },
    { XX,  33,  53,  71,  88, 102, 114, 123, 128, 130, 128, 123, 114, 102,  88,  71,  53,  33,  XX },
    { XX,  34,  54,  73,  90, 105, 117, 126, 132, 134, 132, 126, 117, 105,  90,  73,  54,  34,  XX },
    { XX,  34,  55,  75,  93, 108, 121, 130, 135, 137, 135, 130, 121, 108,  93,  75,  55,  34,  XX },
    { XX,  35,  56,  77,  95, 111, 124, 133, 139, 141, 139, 133, 124, 111,  95,  77,  56,  35,  XX },
    { XX,  36,  58,  79,  98, 114, 127, 137, 143, 145, 143, 137, 127, 114,  98,  79,  58,  36,  XX },
    { XX,  36,  59,  81, 100, 117, 130, 140, 147, 149, 147, 140, 130, 117, 100,  81,  59,  36,  XX },
    { XX,  37,  60,  82, 102, 120, 134, 144, 150, 152, 150, 144, 134, 120, 102,  82,  60,  37,  XX },
    { XX,  38,  62,  84, 105, 123, 137, 148, 154, 156, 154, 148, 137, 123, 105,  84,  62,  38,  XX },
    { XX,  38,  63,  86, 107, 125, 140, 151, 158, 160, 158, 151, 140, 125, 107,  86,  63,  38,  XX },
    { XX,  39,  64,  88, 110, 128, 143, 155, 161, 164, 161, 155, 143, 128, 110,  88,  64,  39,  XX },
    { XX,  39,  66,  90, 112, 131, 147, 158, 165, 168, 165, 158, 147, 131, 112,  90,  66,  39,  XX },
    { XX,  40,  67,  92, 115, 134, 150, 162, 169, 171, 169, 162, 150, 134, 115,  92,  67,  40,  XX },
    { XX,  41,  68,  94, 117, 137, 153, 165, 173, 175, 173, 165, 153, 137, 117,  94,  68,  41,  XX },
    { XX,  41,  69,  96, 119, 140, 157, 169, 176, 179, 176, 169, 157, 140, 119,  96,  69,  41,  XX },
    { XX,  42,  71,  98, 122, 143, 160, 172, 180, 183, 180, 172, 160, 143, 122,  98,  71,  42,  XX },
    { XX,  43,  72,  99, 124, 146, 163, 176, 184, 186, 184, 176, 163, 146, 124,  99,  72,  43,  XX },
    { XX,  43,  73, 101, 127, 149, 166, 180, 188, 190, 188, 180, 166, 149, 127, 101,  73,  43,  XX },
    { XX,  44,  75, 103, 129, 152, 170, 183, 191, 194, 191, 183, 170, 152, 129, 103,  75,  44,  XX },
    { XX,  45,  76, 105, 132, 154, 173, 187, 195, 198, 195, 187, 173, 154, 132, 105,  76,  45,  XX },
    { XX,  45,  77, 107, 134, 157, 176, 190, 199, 202, 199, 190, 176, 157, 134, 107,  77,  45,  XX },
    { XX,  46,  78, 109, 136, 160, 180, 194, 202, 205, 202, 194, 180, 160, 136, 109,  78,  46,  XX },
    { XX,  47,  80, 111, 139, 163, 183, 197, 206, 209, 206, 197, 183, 163, 139, 111,  80,  47,  XX },
    { XX,  47,  81, 113, 141, 166, 186, 201, 210, 213, 210, 201, 186, 166, 141, 113,  81,  47,  XX },
    { XX,  48,  82, 115, 144, 169, 189, 204, 214, 217, 214, 204, 189, 169, 144, 115,  82,  48,  XX },
    { XX,  49,  84, 116, 146, 172, 193, 208, 217, 220, 217, 208, 193, 172, 146, 116,  84,  49,  XX },
    { XX,  49,  85, 118, 149, 175, 196, 212, 221, 224, 221, 212, 196, 175, 149, 118,  85,  49,  XX },
    { XX,  50,  86, 120, 151, 178, 199, 215, 225, 228, 225, 215, 199, 178, 151, 120,  86,  50,  XX },
    { XX,  51,  88, 122, 154, 180, 202, 219, 229, 232, 229, 219, 202, 180, 154, 122,  88,  51,  XX },
    { XX,  51,  89, 124, 156, 183, 206, 222, 232, 236, 232, 222, 206, 183, 156, 124,  89,  51,  XX },
    { XX,  52,  90, 126, 158, 186, 209, 226, 236, 239, 236, 226, 209, 186, 158, 126,  90,  52,  XX },
    { XX,  53,  91, 128, 161, 189, 212, 229, 240, 243, 240, 229, 212, 189, 161, 128,  91,  53,  XX },
    { XX,  53,  93, 130, 163, 192, 216, 233, 243, 247, 243, 233, 216, 192, 163, 130,  93,  53,  XX },
    { XX,  54,  94, 132, 166, 195, 219, 236, 247, 251, 247, 236, 219, 195, 166, 132,  94,  54,  XX },
    { XX,  55,  95, 134, 168, 198, 222, 240, 251, 255, 251, 240, 222, 198, 168, 134,  95,  55,  XX }
};

/**
 * Rend les valeurs PWM para rapport à l'angle spécifié.
 * À appeler lorsque l'angle est connu, c'est à dire, lorsque le moteur
 * est en mouvement.
 * @param alpha Angle, entre 0 et 35.
 * @param puissance, entre 0 et 64.
 * @param ccp Structure pour les valeurs PWM.
 */
void calculeAmplitudesEnMouvement(unsigned char alpha, unsigned char puissance, struct CCP *ccp) {

    // Choisit le tableau de sinus approprié, selon l'amplitude désirée:
    const unsigned char *ssinus = sinus[puissance];

    // Choisit les valeurs des PWM, selon le tableau:
    if (alpha < 12) {
        ccp->ccpa = ssinus[alpha];
        ccp->ccpb = CCPR_MIN;
        ccp->ccpc = ssinus[alpha + 6];
    } else {
        if (alpha < 24) {
            ccp->ccpa = ssinus[alpha - 6];
            ccp->ccpb = ssinus[alpha - 12];
            ccp->ccpc = CCPR_MIN;
        } else {
            ccp->ccpa = CCPR_MIN;
            ccp->ccpb = ssinus[alpha - 18];
            ccp->ccpc = ssinus[alpha - 24];
        }
    }
};

/**
 * Angles moyens par phase.
 */
const unsigned char const angleParPhase[] = {ERROR, 3, 9, 15, 21, 27, 33, ERROR};

/**
 * Rend les valeurs PWM para rapport à la phase spécifiée.
 * À appeler lorsque seule la phase est connue, mais pas l'angle, c'est à dire
 * lorsque le moteur est à l'arret.
 * - Calcule l'angle moyen correspondant à la phase spécifiée.
 * - Utilise 'calculeAmplitudesEnMouvement' pour obtenir les valeurs de
 *   PWM correspondantes à cet angle, avec une puissance moyenne.
 * @param phase Phase, entre 1 et 6.
 * @param ccp Structure pour les valeurs PWM.
 * @param puissance Puissance à utiliser. Il est conseillé de ne pas utiliser
 * une valeur trop forte ici, pour ne pas brûler le circuit.
 */
void calculeAmplitudesArret(unsigned char phase, struct CCP *ccp, unsigned char puissance) {
    if ( (phase == 0) || (phase > 6) ) {
        ccp->ccpa = CCPR_MIN;
        ccp->ccpb = CCPR_MIN;
        ccp->ccpc = CCPR_MIN;
    } else {
        calculeAmplitudesEnMouvement(angleParPhase[phase], puissance, ccp);
    }
}

/*
 * Relation entre valeurs des senseurs Hall et numéro de phase
 */
const unsigned char const phaseParHall[] = {
    0,
    1, 3, 2, 5, 6, 4,
    0
};

/**
 * Determine la phase en cours d'après les senseurs hall.
 * @param hall La valeur des senseurs hall: 0xb*****yzx
 * @return Le numéro de phase, entre 1 et 6.
 */
unsigned char phaseSelonHall(unsigned char hall) {
    // Vérifie que la nouvelle valeur est possible:
    hall = hall & 7;
    if ((hall == 0) || (hall == 7)) {
        return ERROR;
    }

    // Rend le numéro de phase correspondant aux senseurs hall:
    return phaseParHall[hall];
}

/**
 * Calcule la phase en cours à partir de la lecture des senseurs hall.
 * Effectue également un contrôle de la lecture, pour vérifier si elle est
 * possible. Ceci sert à éviter de compter des rebondissements ou du bruit
 * qui affecte la lecture des senseurs.
 * @param hall La valeur des senseurs hall: 0xb*****yzx
 * @param direction Direction actuelle.
 * @return La phase (de 0 à 5) ou un code d'erreur.
 */
unsigned char phaseSelonHallEtDirection(unsigned char hall, enum DIRECTION direction) {

    static unsigned char phase0 = ERROR;
    unsigned char phase;
    signed char step;

    // Obtient la phase selon les senseurs hall:
    phase = phaseSelonHall(hall);

    // Vérifie que la nouvelle valeur est possible, compte tenu de la
    // valeur précédente, et de la direction:
    // (les senseurs hall peuvent avoir des rebondissements)
    if (phase0 != ERROR) {
        step = phase0 - phase;
        switch(step) {
            case -1:
            case 5:
                if (direction == ARRIERE) {
                    return ERROR;
                }
                break;

            case 1:
            case -5:
                if (direction == AVANT) {
                    return ERROR;
                }
                break;

            case 0:
            default:
                return ERROR;
        }
    }

    // Si on arrive ici, la lecture des Hall est considérée possible,
    // et on peut rendre la phase correspondante:
    phase0 = phase;
    return phase;
}

/**
 * Relation entre numéro de phase et angle, lorsque
 * le moteur tourne en avant
 */
const unsigned char const angleParPhaseAvant[] = {ERROR, 0, 6, 12, 18, 24, 30};

/**
 * Relation entre numéro de phase et angle, lorsque
 * le moteur tourne en arrière
 */
const unsigned char const angleParPhaseArriere[] = {ERROR, 6, 12, 18, 24, 30, 0};

/**
 * Calcule l'angle correspondant à la phase et à la direction actuelle
 * de rotation.
 * @param phase Phase actuelle.
 * @param direction Direction actuelle.
 * @return L'angle correspondant.
 */
unsigned char angleSelonPhaseEtDirection(unsigned char phase, enum DIRECTION direction) {

    // Calcule l'angle, selon la phase et la direction:
    switch (direction) {
        case AVANT:
            return angleParPhaseAvant[phase];

        case ARRIERE:
            return angleParPhaseArriere[phase];

        default:
            return ERROR;
    }
}

int eps = 0;
int dureeDeCycle = 0;
unsigned char angleEstime = 0;

/**
 * Cette fonction est appelée en réponse à un changement de phase. À
 * cet instant on connait la valeur exacte des deux paramètres.
 * @param angle Angle exact.
 * @param dureeDePhase Durée de la dernière phase.
 */
void corrigeAngleEtVitesse(unsigned char angle, int dureeDePhase) {
    eps = 0;
    angleEstime = angle;
    dureeDeCycle = dureeDePhase * 6;
}

/**
 * Cette fonction est appelée à chaque cycle de PWM pour calculer (estimer)
 * l'angle actuel.
 * Le calcul se fait sur la base du dernier angle connu avec précision et
 * de la durée de la dernière phase. Ces valeurs ont été établies par l'appel
 * à 'corrigeAngleEtVitesse'.
 * @return L'angle actuel estimé.
 */
unsigned char calculeAngle() {
    eps += CYCLE;
    if (eps > 0) {
        if ( (eps << 1) >= dureeDeCycle )  {
              angleEstime++;
              eps -= dureeDeCycle;
        }
    }
    return angleEstime;
}

#ifdef TEST
unsigned char test_calculeAmplitudesEnMouvement() {
    unsigned char ft = 0;
    struct CCP ccp;

    calculeAmplitudesEnMouvement(0, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpa, TEMPS_MORT, "CCP-00A");
    ft += assertEqualsChar(ccp.ccpb, TEMPS_MORT, "CCP-00B");
    ft += assertEqualsChar(ccp.ccpc,         38, "CCP-00C");

    calculeAmplitudesEnMouvement(3, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpc,         44, "CCP-03C");

    calculeAmplitudesEnMouvement(9, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         44, "CCP-09A");

    calculeAmplitudesEnMouvement(12, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         38, "CCP-12A");
    ft += assertEqualsChar(ccp.ccpb, TEMPS_MORT, "CCP-12B");
    ft += assertEqualsChar(ccp.ccpc, TEMPS_MORT, "CCP-12C");

    calculeAmplitudesEnMouvement(15, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpa,         44, "CCP-15A");

    calculeAmplitudesEnMouvement(21, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpb,         44, "CCP-21B");

    calculeAmplitudesEnMouvement(24, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpa, TEMPS_MORT, "CCP-24A");
    ft += assertEqualsChar(ccp.ccpb,         38, "CCP-24B");
    ft += assertEqualsChar(ccp.ccpc, TEMPS_MORT, "CCP-24C");

    calculeAmplitudesEnMouvement(27, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpb,         44, "CCP-27B");

    calculeAmplitudesEnMouvement(33, 10, &ccp);
    ft += assertEqualsChar(ccp.ccpc,         44, "CCP-32C");

    return ft;
}

unsigned char test_phaseSelonHall() {
    unsigned char ft = 0;

    ft += assertEqualsChar(ERROR, phaseSelonHall(0), "PSH-00");
    ft += assertEqualsChar(1, phaseSelonHall(0b001), "PSH-01");
    ft += assertEqualsChar(2, phaseSelonHall(0b011), "PSH-02");
    ft += assertEqualsChar(3, phaseSelonHall(0b010), "PSH-03");
    ft += assertEqualsChar(4, phaseSelonHall(0b110), "PSH-04");
    ft += assertEqualsChar(5, phaseSelonHall(0b100), "PSH-05");
    ft += assertEqualsChar(6, phaseSelonHall(0b101), "PSH-06");
    ft += assertEqualsChar(ERROR, phaseSelonHall(7), "PSH-07");

    return ft;
}

unsigned char test_calculeAmplitudesArret() {
    unsigned char ft = 0;
    struct CCP ccp;

    calculeAmplitudesArret(0, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpa, TEMPS_MORT, "CAA-0A");
    ft += assertEqualsChar(ccp.ccpb, TEMPS_MORT, "CAA-0B");
    ft += assertEqualsChar(ccp.ccpc, TEMPS_MORT, "CAA-0C");

    calculeAmplitudesArret(1, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpb, TEMPS_MORT, "CAA-1B");
    // Dans le tableau de sinus, TEMPS_MORT represente la valeur zéro.
    ft += assertEqualsChar(2 * ccp.ccpa, ccp.ccpc + TEMPS_MORT, "CAA-1AC");

    calculeAmplitudesArret(2, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpb, TEMPS_MORT, "CAA-2B");
    ft += assertEqualsChar(ccp.ccpa + TEMPS_MORT, 2 * ccp.ccpc, "CAA-2AC");

    calculeAmplitudesArret(3, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpc, TEMPS_MORT, "CAA-3C");
    ft += assertEqualsChar(ccp.ccpa + TEMPS_MORT, 2 * ccp.ccpb, "CAA-3AB");

    calculeAmplitudesArret(4, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpc, TEMPS_MORT, "CAA-4C");
    ft += assertEqualsChar(2 * ccp.ccpa, ccp.ccpb + TEMPS_MORT, "CAA-4AB");

    calculeAmplitudesArret(5, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpa, TEMPS_MORT, "CAA-5A");
    ft += assertEqualsChar(ccp.ccpb + TEMPS_MORT, 2 * ccp.ccpc, "CAA-5BC");

    calculeAmplitudesArret(6, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpa, TEMPS_MORT, "CAA-5A");
    ft += assertEqualsChar(2 * ccp.ccpb, ccp.ccpc + TEMPS_MORT, "CAA-5BC");

    calculeAmplitudesArret(7, &ccp, 15);
    ft += assertEqualsChar(ccp.ccpa, TEMPS_MORT, "CAA-7A");
    ft += assertEqualsChar(ccp.ccpb, TEMPS_MORT, "CAA-7B");
    ft += assertEqualsChar(ccp.ccpc, TEMPS_MORT, "CAA-7C");

    return ft;
}

unsigned char test_phaseSelonHallEtDirection() {
    unsigned char ft = 0;

    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), 1, "PHD-10");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, AVANT), 2, "PHD-20");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b010, AVANT), 3, "PHD-30");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b110, AVANT), 4, "PHD-40");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b100, AVANT), 5, "PHD-50");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b101, AVANT), 6, "PHD-60");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), 1, "PHD-11");

    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), ERROR, "PHD-E0");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, AVANT), 2, "PHD-E1");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, AVANT), ERROR, "PHD-E1");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b001, AVANT), ERROR, "PHD-E3");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b010, AVANT), 3, "PHD-E4");
    ft += assertEqualsChar(phaseSelonHallEtDirection(0b011, ARRIERE), 2, "PHD-E1");

    return ft;
}

unsigned char test_angleSelonPhaseEtDirection() {
    unsigned char ft = 0;

    ft += assertEqualsChar(angleSelonPhaseEtDirection(0, AVANT), ERROR, "CA-001");
    ft += assertEqualsChar(angleSelonPhaseEtDirection(1, AVANT),     0, "CA-002");
    ft += assertEqualsChar(angleSelonPhaseEtDirection(1, ARRIERE),   6, "CA-003");
    ft += assertEqualsChar(angleSelonPhaseEtDirection(2, AVANT),     6, "CA-004");

    return ft;
}

unsigned char test_calculeAngle() {
    unsigned char ft = 0;
    unsigned char n;

    corrigeAngleEtVitesse(0, 40);
    for (n = 0; n < 120; n++) {
        calculeAngle();
    }
    assertEqualsChar(calculeAngle(), 18, "CA-18");

    corrigeAngleEtVitesse(0, 50);
    for (n = 0; n < 120; n++) {
        calculeAngle();
    }
    assertEqualsChar(calculeAngle(), 15, "CA-15");

    corrigeAngleEtVitesse(15, 50);
    for (n = 0; n < 120; n++) {
        calculeAngle();
    }
    assertEqualsChar(calculeAngle(), 30, "CA-15B");

    return ft;
}

/**
 * Point d'entrée pour les tests du moteur.
 * @return Nombre de tests en erreur.
 */
unsigned char test_moteur() {
    unsigned char ft = 0;

    ft += test_phaseSelonHall();
    ft += test_calculeAmplitudesArret();

    ft += test_phaseSelonHallEtDirection();
    ft += test_angleSelonPhaseEtDirection();
    ft += test_calculeAngle();
    ft += test_calculeAmplitudesEnMouvement();

    return ft;
}

#endif