# Technical requirements

* LiPo 2S to 4S
  * At full charge: Vddmax = 4 * 4.2V = 16.8V
  * At minimum charge: Vddmin = 2 * 3.6V = 7.2V
* 100A maximum current
*


# Selection of components

## Power N-MOSFET transistor

As size is very limited, I've chosen a dual MOSFET package. As I want as much
power as I can, and I'm not yet trying to squeeze out the pennies, I've
chosen a rather big component.

Relevant characteristics are:
* https://www.vishay.com/docs/68442/sqjq904e.pdf
* Maximum tolerated gate-to-source: ``Vgsmax`` = 20V (page 1 of data sheet)
* Maximum tolerated drain-to-source: ``Vdsmax`` = 40V (page 1 of data sheet)
* Minimum gate-to-source to have good conduction: ``Vgsmin`` = 5V (page 3 of data sheet)
* Gate charge ``Qg`` = 75nC

## Capacitors
To minimise the number of different components (still not trying to squeeze out
the pennies, but it makes the hand assembly easier when you've got less different
pieces), I'm choosing KEMET's X7R series, which are not very expensive, and are
appropriate for bypass and decoupling applications.

Also, I'm soldering by hand, so I don't want to use a package smaller
than C0402.

* https://content.kemet.com/datasheets/KEM_C1002_X7R_SMD.pdf
* Insulation resistance for capacitors above 0.012µF:
  * ``IR``= 500 MΩ µF
  * or 10GΩ
  * _To obtain IR limit, divide MΩ - μF value by the capacitance and compare to GΩ limit_
  * Page 13

## High / Low side MOSFET driver

I'm quite familiar with the IR2301SPBF‎, which has several advantages:

* It is very common.
* It is expensive, but not too much.
* It is available as DIP and also SMD.
* It supports small Vcc.

# Circuit design

## Bootstrap capacitor

To calculate the value of the bootstrap capacitor, one must understand
how the high side / low side driver works.

![Hi side / low side driver bootstrap capacitor](documentation/hi-low-side-mosfet-driver-bootsatrap-capacitor.png)

For the high side transistor to be on, the gate has to be on a higher potential
than the source:

<img src="/tex/9c1f7859541d779cdf23e6e9ec9476f0.svg?invert_in_darkmode&sanitize=true" align=middle width=130.4040243pt height=22.465723500000017pt/>

While the low side is in conduction, ``Vs`` is at ground. The capacitor _Cboot_ is
charged at Vcc through the diode _Dboot_ (minus its forward voltage):

<img src="/tex/b6018c4e729526ded1921fd2f182e137.svg?invert_in_darkmode&sanitize=true" align=middle width=158.69965484999997pt height=22.465723500000017pt/>

When the low side stops conducting, what happens to the potential at point ``A``
depends on what the other bridges and the rest of the circuit do. What is certain is,
because the _Cboot_ cannot discharge itself due to the _Dboot_ diode, ``Vb``will
follow it on top of the capacitor:

<img src="/tex/8215727317237c45698142b5505bd60c.svg?invert_in_darkmode&sanitize=true" align=middle width=114.92940689999999pt height=22.465723500000017pt/>

The potential in ``Vb`` feeds the internal circuitry that produces the ``Ho`` output. When
the moment comes, ``Ho`` can make the high side to enter in conduction provided that:

<img src="/tex/9540094d9cf385b732f69eaa9f100150.svg?invert_in_darkmode&sanitize=true" align=middle width=173.26638284999999pt height=22.465723500000017pt/>

This condition has to hold as long as the high side is on. During this period,
let's call it ``thon``, the only source of power is the charge in _Cboot_, and
it has to provide for:
* Leaking currents:
  * _Dboot_ diode's leaking:
  * Transistor's gate leaking:
  * _Cboot_ leaking through the insulation resistance:
  * Current consumed by the internal circuitry that produces the ``Ho`` signal:
* Gate charges:Charging the gate of the high side transistor:
  * ``Qg`` = 75nC
