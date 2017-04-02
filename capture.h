#ifndef CAPTURE__H
#define	CAPTURE__H

void captureFlancMontant(unsigned char canal, unsigned int instant);
unsigned char captureFlancDescendant(unsigned char canal, unsigned int instant);

#ifdef TEST
unsigned char test_capture();
#endif

#endif

