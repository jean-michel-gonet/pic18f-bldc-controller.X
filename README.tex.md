# Technical requirements

* LiPo 2S to 4S
  * At full charge: Vddmax = 4 * 4.2V = 16.8V
  * At minimum charge: Vddmin = 2 * 3.6V = 7.2V
* 100A maximum current
* 128kHz PWM frequency.

## Why not higher number of LiPo cells?

RC Car races are regulated by the ROARR, which states that electric RC cars
should use 2S only. Therefore 4S is already out of norm, which means that
most model car engines wont't hold more than 4S.

## Why 100A?
Because it is the golden dream of any RC car owner. To be honest, don't really
believe that this small circuit will hold that amount of current. Something
is going to break or melt before. But still, let it be a idealistic objective.

## Why 128kHz?
According to [AN885], page 8, _«As a rule of thumb, the PWM frequency should be
at least 10 times that of the maximum frequency of the motor»_.

A very high KV constant for standard ROAR-compliant Brushless RC car motor is
around 6100KV for 2S, which means:
* 6100 rpm
* per volt
* 7.4 volts nominal.
* 6 windings.

A brushless motor

$ V_r = \frac{6100 \cdot 7400}{60} \cdot 6 = 45 kHz$

Just to be on the safe side, let's multiply it by two: 90 kHz. To reach this
frequency, the PIC18 has to be clocked at 64MHZ, so the highest reasonable
PWM frequency it can generate is 128kHZ, which keeps a very safe margin with
the value calculated above.

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
* Gate-source leakage: ``Igss`` = 100nA

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

## Diodes
Because I'm familiar with it, its high availability, and acceptable forward
voltage drop, I'm choosing the BAT48ZFILM:

* https://www.st.com/content/ccc/resource/technical/document/datasheet/b2/2f/2a/0c/8b/b5/46/a3/CD00130229.pdf/files/CD00130229.pdf/jcr:content/translations/en.CD00130229.pdf
* Forward voltage drop:
  * At 0.1mA: ``Vf`` = 0.25V
* Reverse leakage current:
  * At 20V: ``Ilk`` = 5µA

## High / Low side MOSFET driver

I'm quite familiar with the IR2301SPBF‎, which is commonly available, not too
expensive, available as DIP and SMD and, most importantly, it supports small
``Vcc``:

* https://www.infineon.com/dgdl/ir2301.pdf?fileId=5546d462533600a4015355c97bb216dc
* VCC and VBS supply under-voltage going threshold: ``Vccuv`` = 5V
* Quiescent VBS supply current: ``Iqbs`` = 100µA

# Circuit design

I've detailed here some of the calculation I did to estimate the values of certain
components.

## Breakpoint protection

The module is aimed at being something open to tinkering. This means that it
will often run in debug mode. In this mode, the micro-controller can be stopped
at any point by a breakpoint, freezing the PWM signal in high status. When
this happen, the current through the motor will raise exponentially until
something burns or the battery explodes.

To prevent this, a high pass filter guards the high side input of the driver:

If the PWM is frozen high, the capacitor will charge, the current in the resistor
will drop to zero, and so the ``HIN`` entry.

Let's size the RC circuit so its time constant is 5 times the period of the
PWM signal:

$ \tau = R \cdot C = 5 \cdot T_{pwm} = \frac{5}{F_{pwm}}$

Let's chose arbitrarily a capacitor of 10nF. The frequency of the PWM signal
is 128kHz.

$R = \frac{5}{C \cdot F_{pwm}} = \frac{5}{10 \cdot 10^{-9} \cdot 128 \cdot 10^{3}} = 39062\Omega$

To be
## Bootstrap capacitor

To calculate the value of the bootstrap capacitor, one must understand how the
high side / low side driver works.

![Hi side / low side driver bootstrap capacitor](documentation/hi-low-side-mosfet-driver-bootsatrap-capacitor.png)

For the high side transistor to be on, the gate has to be on a higher potential
than the source:

$V_g > V_{dd} + V_{gsmin}$

While the low side is in conduction, ``Vs`` is at ground. The capacitor _Cboot_ is
charged at Vcc through the diode _Dboot_ (minus its forward voltage):

$V_{cboot}(t_0) = V_{cc} - V_{FDboot}$

When the low side stops conducting, what happens to the potential at point ``A``
depends on what the other bridges and the rest of the circuit do. What is certain is,
because the _Cboot_ cannot discharge itself due to the _Dboot_ diode, ``Vb``will
follow it on top of the capacitor:

$V_b = V_{cboot} + V_a$

The potential in ``Vb`` feeds the internal circuitry that produces the ``Ho`` output. When
the moment comes, ``Ho`` can make the high side to enter in conduction provided that:

$V_{cboot} > V_{gsmin}$

This condition has to hold as long as the high side is on. At a frequency of
128kHz, considering an unrealistic duty cycle of 100%, the maximum time the
high side needs to keep conducting is:

$t_{hon} < \frac{1}{125 \cdot 10^3} = 7.8 \mu s$

During this period, the only source of power is the charge stored in _Cboot_, and
it has to provide for:

* Leaking currents:
  * _Cboot_ leaking through the insulation resistance:
    * Assuming a capacitor of 0.1µA (can correct that later)
    * ``IRcboot`` = 500 MΩ / 0.1µA = 5GΩ
    * At 10V: ``ILKCboot`` = 2nA
  * _Dboot_ diode's reverse leaking current: ``ILKDboot`` = 5µA
  * Transistor's gate leaking: ``Igss`` = 100nA
  * Current consumed by the internal circuitry that produces the ``Ho`` signal:
    * Quiescent VBS supply current: ``Iqbs`` = 100µA
* Gate charges:Charging the gate of the high side transistor:
  * ``Qg`` = 75nC

The charge lost by _Cboot_ by the end of the ``thon`` period is the sum of all
of the above:

$Q_{boot} = Q_g + t_{hon} \cdot (I_{LKCboot} + I_{LKDboot} + I_{gss} + I_{qbs})$

By comparing at the different leakage currents, the only significant one
is the quiescent Vbs supply current, 100µA, which is 20 times greater than the
next in significance, which is the diode reverse leaking current, 5µA. So we can
say:

$Q_{boot} = Q_g + t_{hon} \cdot I_{qbs}$

When replacing actual values, we see again that none of that counts but
the transistor's gate charge:

$Q_{boot} = 75nC + 7.8 \mu s \cdot 100 \mu A = 75nC$

Because of the ``Qboot`` charge lost during the high on time, by the
end of the high on period, the voltage in _Cboot_ will be:

$V_{cboot}(t_{hon}) = V_{cboot}(t_0) - \frac{Q_{boot}}{C_{boot}}$

And this has to be enough to keep the high side transistor on:

$V_{cboot}(t_0) - \frac{Q_{boot}}{C_{boot}} > V_{gsmin}$

Replacing the values to have the complete expression, gives us:

$V_{cc} - V_{FDboot} - \frac{Q_{boot}}{C_{boot}} > V_{gsmin}$

Operating we can come up with:

$C > \frac{Q_g}{V_{cc} - V_{FDboot} - V_{gsmin}}$

And thus:

$C > \frac{75nC}{7.2V - 0.25V - 5V} = 39nF$

Taking in count the tolerance of 10%, plus some margin, we can safely use
0.1µF capacitors for the bootstrap circuit.

# Bibliography

## References
* [AN885] - _Brushless DC (BLDC) Motor Fundamentals_
  * Describes how a brushless motor works.
  * Microchip, 2003.
  * http://ww1.microchip.com/downloads/en/AppNotes/00885a.pdf


## Additional bibliography
Bibliography I consulted to design the circuit.

* **Brushless Motor Kv Constant Explained**
  * A great document about what is KV constant, and plenty of other related
    concepts that you need to know.
  * http://learningrc.com/motor-kv/
* **SMT / SMD Components & packages, sizes, dimensions, details**
  * A great list of package sizes. Package sizes are critical when you're
    soldering SMD by hand, because some of them are so small you can
    barely see them, let alone soldering them.
  * https://www.electronics-notes.com/articles/electronic_components/surface-mount-technology-smd-smt/packages.php
* **Using Monolithic High Voltage Gate Drivers**
  * Application note from Internation Rectifier about high side drivers.
    Although it is mainly directed at IGBT, most calculations translate
    directly to MOSFET.
  * https://www.infineon.com/dgdl/Infineon-Using_Monolithic_Voltage_Gate_Drivers-AN-v01_00-EN.pdf?fileId=5546d462584d1d4a01585242c11947b1
* **XERUN 3652/3660 G2, HOBBYWING User Manual**
  * The user manual of the G2 series by Hobbywing shows some common characteristics
    of the brushless motors this module is aiming at.
  * https://cdn.shopify.com/s/files/1/0109/9702/files/XeRun36523660SDG2.pdf?13512914181997608195
