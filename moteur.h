#ifndef __MOTEUR_H
#define __MOTEUR_H

#include "domaine.h"

/**
 * Contient les valeurs des rapports cycliques des trois
 * modulations PWM.
 */
struct CCP {
    /** Rapport cyclique pour le connecteur de la bobine A. */
    unsigned char ccpa;
    /** Rapport cyclique pour le connecteur de la bobine B. */
    unsigned char ccpb;
    /** Rapport cyclique pour le connecteur de la bobine C. */
    unsigned char ccpc;
};


/**
 * Rend les valeurs PWM para rapport à l'angle spécifié.
 * À appeler lorsque l'angle est connu, c'est à dire, lorsque le moteur
 * est en mouvement.
 * @param alpha Angle, entre 0 et 35.
 * @param puissance, entre 0 et 64.
 * @param ccp Structure pour les valeurs PWM.
 */
void calculeAmplitudesEnMouvement(unsigned char alpha, unsigned char puissance, struct CCP *ccp);

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
void calculeAmplitudesArret(unsigned char phase, struct CCP *ccp, unsigned char puissance);

/**
 * Determine la phase en cours d'après les senseurs hall.
 * @param hall La valeur des senseurs hall: 0xb*****yzx
 * @return Le numéro de phase, entre 1 et 6.
 */
unsigned char phaseSelonHall(unsigned char hall);

/**
 * Calcule la phase en cours à partir de la lecture des senseurs hall.
 * Effectue également un contrôle de la lecture, pour vérifier si elle est
 * possible. Ceci sert à éviter de compter des rebondissements ou du bruit
 * qui affecte la lecture des senseurs.
 * @param hall La valeur des senseurs hall: 0xb*****yzx
 * @param direction Direction actuelle.
 * @return La phase (de 0 à 5) ou un code d'erreur.
 */
unsigned char phaseSelonHallEtDirection(unsigned char hall, enum DIRECTION direction);

/**
 * Calcule l'angle correspondant à la phase et à la direction actuelle
 * de rotation.
 * @param phase Phase actuelle.
 * @param direction Direction actuelle.
 * @return L'angle correspondant.
 */
unsigned char angleSelonPhaseEtDirection(unsigned char phase, enum DIRECTION direction);

/**
 * Cette fonction est appelée en réponse à un changement de phase. À
 * cet instant on connait la valeur exacte des deux paramètres.
 * @param angle Angle exact.
 * @param dureeDePhase Durée de la dernière phase.
 */
void corrigeAngleEtVitesse(unsigned char angle, int dureeDePhase);

/**
 * Cette fonction est appelée à chaque cycle de PWM pour calculer (estimer)
 * l'angle actuel.
 * Le calcul se fait sur la base du dernier angle connu avec précision et
 * de la durée de la dernière phase. Ces valeurs ont été établies par l'appel
 * à 'corrigeAngleEtVitesse'.
 * @return L'angle actuel estimé.
 */
unsigned char calculeAngle();

#ifdef TEST
/**
 * Point d'entrée pour les tests du moteur.
 * @return Nombre de tests en erreur.
 */
unsigned char test_moteur();
#endif

#endif