.. _ch-ieee754:

********************************
Binary floating-point arithmetic
********************************

This chapter is a short introduction to the used notation and important aspects
of the binary floating-point arithmetic as defined in the most recent
:term:`IEEE 754-2008`. A more comprehensive introduction, including non-binary
floating-point arithmetic, is given in [Brisebarre2010]_ (Chapters 2 and 3).



Notation and encoding
=====================

In this project only the binary 64 bit floating-point format is regarded. In
the floating-point standard it is called :term:`binary64`. The C and C++ data
type *double* realize this format as well, see [ISO-IEC-9899-2011]_ (Annex F)
and [ISO-IEC-14882-2011]_ (Chapter 18.3). The set of normal and subnormal
:term:`binary64` floating-point numbers :math:`x \in \mathbb{F}` is defined by
the precision *p = 53* and the exponent constant *E = 1023*.

.. math::
   x =
   \begin{cases}
   \pm \; 1.m_{1}m_{2} \dots m_{p-1} \cdot 2^{e},\quad - E + 1 \leq e \leq E
   & \text{normal} \\
   \pm \; 0.m_{1}m_{2} \dots m_{p-1} \cdot 2^{-E}
   & \text{subnormal}
   \end{cases}

Next to normal and subnormal numbers, a :term:`binary64` can take the special
values :math:`\pm \infty` and *NaN* [Brisebarre2010]_ (Chapter 3). For this
project the :term:`binary64` encoding plays an important role, e.g. for the
exponent extraction (see Chapter :ref:`sec-Fast exponent extraction`). This
encoding is shown in Figure :ref:`fig-Encoding of a binary64 floating-point
number` with the most significant sign bit, an eleven bit exponent and a 52 bit
significant [IEEE-754-2008]_ (Chapter 3.4).

.. figure:: _static/ch02-binary64.*
   :alt: Encoding of a binary64 floating-point number.
   :align: center
   :name: fig-Encoding of a binary64 floating-point number

   Encoding of a :term:`binary64` floating-point number.

To get an idea of the range of :term:`binary64` numbers, some useful macro
constants of the *double* data type from the C standard library header
*<float.h>* [ISO-IEC-9899-2011]_ (Chapter 5.2.4.2.2) are listed in Table
:ref:`tbl-Some double macro constants from float.h`.

.. list-table:: Some *double* macro constants from *<float.h>*
   :header-rows: 1
   :name: tbl-Some double macro constants from float.h

   * - Marco name
     - Value
     - Description
   * - DBL\_MIN
     - :math:`2^{-1022}`
     - Smallest positive normal number
   * - DBL\_TRUE\_MIN
     - :math:`2^{-1074}`
     - Smalles positive subnormal number
   * - DBL\_MAX
     - :math:`\texttt{0x1.fffffffffffff} \cdot 2^{1023}`
     - Largest positive normal number
   * - DBL\_EPSILON
     - :math:`2^{-52}`
     - Relative unit roundoff



.. _sec-Rounding:

Rounding
========

:term:`IEEE 754-2008` requires the results of conforming floating-point
operations to be correctly rounded, see  Definition 2.1.12 and Chapter 9 in
[IEEE-754-2008]_. The following description is a brief summary of
[Brisebarre2010]_ (Chapter 2.2) to explain a correctly rounded floating-point
number in the context of a given rounding mode. Assume a general positive,
normalized, binary, infinite precise floating-point number :math:`y \in
\mathbb{R}`. Then the first *p* bits are the representable bits (remember for
:term:`binary64` the precision *p* is 53), followed by a rounding bit and the
remaining bits results in the sticky bit. The sticky bit is set, if any bit of
:math:`m_{p+1}m_{p+2} \dots` is set.

.. math::
   y = \underbrace{1.m_{1}m_{2} \dots m_{p-1}}_{\text{representable bits}}
   \underbrace{m_{p}}_{\text{round bit}}
   \underbrace{m_{p+1}m_{p+2} \dots}_{\text{sticky bit}} \cdot 2^{e},
   \quad -\infty < e < \infty

In order to be representable as :term:`binary64`, *y* has to be rounded to a
floating-point number :math:`x \in \mathbb{F}`. This rounding operation will be
defined by the function *x = fl(y)*. The result of *fl()* is dependent on
the active rounding mode. The :term:`IEEE 754-2008` specifies the following
rounding modes:

* roundToNearest

  * roundTiesToEven
  * roundTiesToAway

* roundTowardPositive
* roundTowardNegative
* roundTowardZero

For a binary :term:`IEEE 754-2008` implementation *roundTiesToEven* is the
default rounding mode, though the other three rounding modes have to be
implemented as well, see Chapter 4.3.3 [IEEE-754-2008]_. Depending on the sign
of *y*, *roundTowardZero* behaves like *roundTowardPositive* or
*roundTowardNegative* respectively. This rounding mode will be neglected in the
following examinations. Table :ref:`tbl-Comparison of rounding modes` (according
to [Brisebarre2010]_, Chapter 2.2.1) gives an overview how the different
rounding modes work. "-" indicates that the significant of *y* is simply
truncated after :math:`m_{p-1}`. "p" means that :math:`2^{-p+1}` has to be added
to the significant, that is truncated after bit :math:`m_{p-1}`.

.. csv-table:: Comparison of rounding modes
   :header: "round bit", "sticky bit", "roundTowardNegative", "roundTowardPositive", "roundTiesToEven"
   :name: tbl-Comparison of rounding modes

   0, 0, "--", "--", "--"
   0, 1, "--", "p", "--"
   1, 0, "--", "p", "--"
   1, 1, "--", "p", "p"

Thus an infinite precise result of an arithmetic operation *y* is correctly
rounded according to a rounding mode, if the rules above are applied.  This
definition is intangible for mathematical proofs on error estimations of
floating-point operations. To overcome this problem, several measures have been
introduced. The first one is the relative unit roundoff ε, especially for
:term:`binary64` holds *ε = DBL_EPSILON*. With this measure a correctly rounded
result *x = fl(y)* according to [Brisebarre2010]_ (Chapter 2.2.3) can be defined
as:

.. math::
   \dfrac{\vert y - x \vert}{\vert y \vert}
   \begin{cases}
   \leq 0.5\epsilon, & \text{for roundToNearest} \\
   < \epsilon, & \text{otherwise.}
   \end{cases}

This more intuitive measure for the term correctly rounded means, that for any
rounding mode the relative error should be smaller than the representable bits
of the rounded floating-point result *x*. Additionally for roundToNearest the
relative error can be maximal exactly the tie value *0.5 · ε* (the round bit of
*y*), or is "one bit smaller" than for any other rounding mode.

A measure for the absolute error is the *unit in the last place*
:math:`ulp(y),\; y \in \mathbb{R}`, which is defined for any real number in
[Brisebarre2010]_ (Definition 5) as:

.. math::
   ulp(y) = max(2^{e-p+1}, DBL\_TRUE\_MIN)
   \qquad y \in \left[2^{e}, \; 2^{e + 1}\right)

ε and *ulp(y)* are related via *ulp(1) = ε*, see [Brisebarre2010]_ (Chapter
2.6.4). Roughly spoken, *ulp(x)*, :math:`x \in \mathbb{F}` is the significance
of bit :math:`m_{p-1}` of *x* or reciting the original definition:

   "*ulp(x)* is the gap between the two floating-point numbers nearest to *x*,
   even if *x* is one of them."

   -- [Brisebarre2010]_ (p. 32)

When there is a *unit in the last place*, it seems likely that there exists a
*unit in the first place* as well. Rump, Ogita, and Oishi introduce *ufp(y)*
in [Ogita2008]_:

.. math::
  ufp(y) =
  \begin{cases}
  2^{\lfloor \log_{2} \lvert y \rvert \rfloor},
  & \text{for } y \in \mathbb{R} \setminus \{ 0 \} \\
  0, & \text{for } y = 0.
  \end{cases}

Both ε and *ufp(y)* are often found in literature about error analysis of
floating-point algorithms in various forms and have some weaknesses in their
informative value. Discussions about the limitations can be found in
[Brisebarre2010]_ (Chapter 2) and with emphasis on *ufp(y)* in [Rump2012]_.



.. _sec-Fast exponent extraction:

Fast exponent extraction
========================

A crucial operation for the later proposed algorithms is to extract the exponent
of a :term:`binary64`. The standard C library offers two functions to extract
the exponent part, namely *frexp()* and *ilogb()*, see [ISO-IEC-9899-2011]_
(Chapter 7.12.6) and [ISO-IEC-14882-2011]_ (Chapter 26.8) for details.
Additionally two more hardware dependent methods were tested too. The first of
these is to define a data structure, that allows to conveniently view certain
bit positions of a :term:`binary64` as unsigned integers, see Listing
:ref:`lst-Exponent extraction via type conversion`. This approach has also been
chosen for the algorithm implementations in [Hayes2010]_. As done in the last
line of Listing :ref:`lst-Exponent extraction via type conversion`, the
:term:`binary64` has to be cast to the structure in order to extract the
exponent. This approach is called "bit ops" in Figure :ref:`fig-Exponent
extraction performance` as it performs operations on the bit level
representation of the :term:`binary64`.

.. code-block:: c
   :caption: Exponent extraction via type conversion
   :name: lst-Exponent extraction via type conversion
   :linenos:

   typedef struct {
     unsigned mantissa_low:32;
     unsigned mantissa_high:20;
     unsigned exponent:11;
     unsigned sign:1;
   } ieee754_double;

   /* ... */

   double d = 1.0;
   unsigned exponent = ((ieee754_double *) &d)->exponent;

The last considered approach is to use inline assembly to directly call the
assembler instruction *fxtract* wrapped by a function with an interface similar
to that one of *frexp()*, see Listing :ref:`lst-Exponent extraction via inline
assembly`.

.. code-block:: c
   :caption: Exponent extraction via inline assembly
   :name: lst-Exponent extraction via inline assembly
   :linenos:

   inline 
   double asm_fxtract (const double input, int *exponent) {
     double result = 0.0;
     double exp = 0.0;
     __asm__ ("fxtract": "=t" (result), "=u" (exp):"0" (input));
     *exponent = (int) exp;
     return result;
   }

To compare these four approaches for extracting the exponent of a
:term:`binary64`, a small benchmark program has been created. The benchmark
program performs the extraction operation on a varying number of input operands
and repeats for each number of operands the action 100 times. This has been done
to receive measurable results, as this operation is performed too fast to obtain
reliable results for a small amount of input. The timings of the four methods
have been compared relative to the time needed by a simple addition operation on
the same amount of input data, as visible in Figure :ref:`fig-Exponent
extraction performance`. The choice of the addition operation as reference value
is not deciding. Figure :ref:`fig-Exponent extraction performance` reveals
almost equal timings for the multiplication operation. Because of this, the
multiplication plot is hidden behind the addition plot in Figure
:ref:`fig-Exponent extraction performance`. A final remark on the benchmark
program is the instruction-level parallelism. The benchmark was performed with
one to four parallel instructions by a technique called "partial loop
unrolling", which is explained later in Chapter :ref:`sec-Performance`.

Figure :ref:`fig-Exponent extraction performance` shows, that type casting ("bit
ops") performs best for extracting the exponent and might have been chosen
intentionally for the algorithm implementations in [Hayes2010]_. Even for the
parallel case this approach doesn't perform worse than an addition or
multiplication operation, what is beneficial if one is able to parallelise this
task for the input data. One drawback of this approach is, that like the
assembler instruction *fxtract*, it is hardware depended. Thus a user of the
presented algorithms has to take care for the data structure of Listing
:ref:`lst-Exponent extraction via type conversion` to be applicable to his
machine. If the user favours generality over performance, the standard library
function *frexp()* should be chosen for the task of exponent extraction.

.. figure:: _static/benchmark_decomposition.*
   :alt: Exponent extraction performance.
   :align: center
   :name: fig-Exponent extraction performance

   Exponent extraction performance compared to the add operation.

