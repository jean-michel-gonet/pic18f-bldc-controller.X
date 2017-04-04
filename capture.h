#ifndef CAPTURE__H
#define	CAPTURE__H

void captureFlancMontant(unsigned char canal, unsigned int instant);
unsigned char captureFlancDescendant(unsigned char canal, unsigned int instant);

#ifdef TEST
void test_capture();
#endif

#endif

