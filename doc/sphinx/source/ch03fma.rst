.. _ch-fma:

******************************** 
The Fused-Multiply-Add operation
********************************

:term:`IEEE 754-2008` standardizes the new operation :term:`FMA` \cite[Page
4]{IEEE2008}. This operation will be supported by hardware in upcoming
:term:`CPU` instruction sets from AMD [#f1]_
\cite[Page 3]{Hollingsworth2012} and Intel [#f2]_
\cite[Page 4]{Meghana2013}, which at the time of writing are two market
leading :term:`CPU` manufactures. What makes this instruction so attractive is,
that it performs two floating-point operations in a single step [#f3]_, which
can be beneficial for speeding up algorithms. A short history excerpt and a very
comprehensive analysis of various algorithms using the :term:`FMA` operation
can be found in \cite[Chapter 5]{Muller2010}.



Specification
=============

This section deals with how :term:`FMA` is specified by :term:`IEEE 754-2008`.
Quoting the standard:

   "The operation *fusedMultiplyAdd(x, y, z)* computes
   :math:`(x \cdot y ) + z` as if with unbounded range and precision,
   rounding only once to the destination format."

   --\cite[Definition 2.1.28]{IEEE2008}

.. code-block:: octave
   :caption: :term:`FMA`
   :name: alg-FMA
   :linenos:

   function FMA(a,b,c)
     return fl((a · b) + c)
   end function

The property, that the result of the :term:`FMA` operation is only rounded
once in the end, is very important. First of all, one is able to write
algorithms that can rely on this property of no intermediate rounding like
it would happen if :math:`(x \cdot y ) + z` is evaluated by two floating-point
operations. But on the other hand the programmer has to take care for
compiler optimizations. For example an expression like
:math:`a \cdot b + x \cdot y`
might be contracted to *FMA(a, b, x · y)* and any assumptions about
intermediate rounding are falsified \cite[Page 214]{Muller2010}. More on
the compiler settings for correct usage for :term:`FMA` follows in the Section
:ref:`sec-Software and compiler support`.



.. _sec-Hardware realization:

Hardware realization
====================

At the time of writing this thesis the :term:`FMA` operation is not a fully
adopted by all common used computer architectures. Thus, it is important to
check if the used system uses a hardware implemented :term:`FMA` operation in
order to avoid slow software emulations as the :term:`C11` standard remarks
\cite[Chapter 7.12]{ISO/IEC2011a}.

Since the year 1990 the :term:`FMA` instruction has been supported
by several processors, like the *HP/Intel Itanium*, which has
been used as testing system by many algorithm implementors in the past
\cite[Page 151]{Muller2010}. For examples see \cite[Page 8]{Rump2005} and
\cite{Graillat2007}. These processors are mainly used for scientific or
business applications, but the focus of this thesis are more common used
processor architectures like the Intel 64 and AMD64 [#f4]_ architectures,
which are mainly compatible with each other. AMD
assures that "[...] floating-point operations comply with the IEEE-754
standard." \cite[Chapter 1.1.5]{AMD2013}, as Intel does for :term:`FMA`
too \cite[Chapter 14.5.1]{Intel2013}. So it it assumed for this thesis,
that there is no violation of :term:`IEEE 754-2008` on hardware level.

\begin{figure}
\begin{bytefield}[bitwidth=0.4em,endianness=big,rightcurly=.]{80}
  \bitlabel{1}{Sign} \\ \bitheader{0,63,79} \\ \begin{rightwordgroup}{R0}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX0}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{R1}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX1}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{R2}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX2}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{R3}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX3}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{R4}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX4}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{R5}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX5}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{R6}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX6}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{R7}
    \bitbox{1}{} & \bitbox{15}{Exponent} & \colorbitbox{Gray}{64}{Significand
    / MMX7}
  \end{rightwordgroup} \\
\end{bytefield} \caption{x87 FPU and mapped MMX registers.} \label{fig:x87
FPU and mapped MMX registers} \end{figure}

\begin{figure}
\begin{bytefield}[bitwidth=0.1em,endianness=big,rightcurly=.]{256}
  \bitheader{0,64,128,192,255} \\ \begin{rightwordgroup}{YMM2}
    \bitbox{64}{double 1.1} & \bitbox{64}{double 1.2} & \bitbox{64}{double
    1.3} & \bitbox{64}{double 1.4}
  \end{rightwordgroup} \\
  \makebox[25.6em][c]{+\hspace*{25mm}+\hspace*{25mm}+\hspace*{25mm}+} \\\\
  \begin{rightwordgroup}{YMM3}
    \bitbox{64}{double 2.1} & \bitbox{64}{double 2.2} & \bitbox{64}{double
    2.3} & \bitbox{64}{double 2.4}
  \end{rightwordgroup} \\
  \makebox[25.6em][c]{=\hspace*{25mm}=\hspace*{25mm}=\hspace*{25mm}=} \\\\
  \begin{rightwordgroup}{YMM1}
    \bitbox{64}{double 3.1} & \bitbox{64}{double 3.2} & \bitbox{64}{double
    3.3} & \bitbox{64}{double 3.4}
  \end{rightwordgroup}
\end{bytefield} \caption[AVX packed double addition.]
  {AVX packed double addition *VADDPD ymm1, ymm2, ymm3/m256}*.}
\label{fig:AVX packed double addition} \end{figure}

For analyzing the :term:`FMA` operation on hardware level, a deeper understanding
of the floating-point instruction sets and used registers is required. With
this knowledge one can later check on the assembly level, if the "real"
:term:`FMA` is used. Many currently available and all upcoming Intel 64
and AMD64 :term:`CPU` s support four floating-point instruction sets:

* x87 :term:`FPU` : This instruction set is
  designed for scalar :term:`IEEE 754-1985` floating-point operations on eight
  separate 80 bit :term:`FPU` data registers. These registers are used as a
  stack, to avoid long opcodes. The mnemonics are prefixed by an "F" (float),
  for example *FADD ST(0),ST(i)* is such an instruction. *ST(x)*
  is the stack pointer to some :term:`FPU` data register *x*. The shown
  instruction replaces *ST(0)* with *ST(0) + ST(i)*. An urgent
  problem with these instructions arises from the 80 bit long registers and
  when, due to execution optimization, floating-point operands are kept in the
  registers for more than one operation. This problem of "double rounding"
  is described in \cite[Chapters 3.3.1]{Muller2010}. For more information
  about the x87 :term:`FPU`, see \cite[Chapter 8]{Intel2013} and
  \cite[Chapter 6]{AMD2013}).

* :term:`MMX` and 3DNow! [#f5]_ :
  In contrast to the x87 :term:`FPU` instruction
  set, this one is intended for :term:`SIMD` operations. Intel supports only
  integer data types, whereas AMD introduced the extension *3DNow!* for
  supporting floating-point data types as well. This instruction set makes use
  of eight 64 bit :term:`MMX` data registers that are mapped onto the :term:`FPU`
  data registers (see Figure :ref:`fig-x87 FPU and mapped MMX registers`). So
  instead of having two scalar operands for an instruction, :term:`MMX` allows to
  operate on so called packed values, which are vectors of some data type (see
  Figure :ref:`fig-AVX packed double addition`). For AMD only, the mnemonics are
  prefixed with "PF" (packed float), for example *PFADD mmx1, mmx2/mem64*
  would add the packed values from two :term:`MMX` data registers and store the
  results in the first operand register. Due to the 64 bit limitation, only two
  packed single precision floating-point types could be used (see \cite[Page
  94]{AMD2013d} for more details). \cite[Chapter 9]{Intel2013} \cite[Chapter
  5]{AMD2013} 

* Legacy :term:`SSE` [#f6]_:
  To legacy :term:`SSE` belong SSE1, SSE2, SSE3, SSSE3,
  SSE4, SSE4.1, SSE4.2 and SSE4A. All these instruction sets are subsequent
  extensions with new instructions. The difference to :term:`MMX` is that :term:`SSE`
  instructions can operate on both :term:`MMX` (thus :term:`FPU`) data registers and
  eight to sixteen 128 bit *XMM* data registers depending on enabled 64 bit
  mode. The packed value concept stays the same as in :term:`MMX`, but offers more
  operands at the same time and the support of floating-point types. An example
  for adding two packed double precision floating-point types is *ADDPD
  xmm1, xmm2/mem128*, like with *PFADD* the result is stored in the first
  operand register \cite[Page 19]{AMD2013c}. The instruction suffix "SD"
  thus indicates a scalar double-precision operation and the suffix "PD"
  a packed double operation. \cite[Chapter 10-12]{Intel2013} \cite[Chapter
  4]{AMD2013}).

* Extended :term:`SSE`: This instruction set is
  the most interesting one for this thesis, as it will include the :term:`FMA`
  operation. To extended :term:`SSE` belongs the :term:`AVX` instruction set,
  that offers 256 bit versions of all legacy :term:`SSE` instructions and
  further extensions, that are manufacturer depended and not relevant for
  this thesis. Sixteen 256 bit *YMM* data registers, whose 128 lower bits
  are used as *XMM* data registers (see Figure :ref:`fig-SSE registers`)
  are required to perform these extended operations. The mnemonics are
  prefixed by "V" (VEX-prefix) \cite[Chapter 14.1.3]{Intel2013}. There is
  one important difference between Intel and AMD. Both will have support for
  the so called :term:`FMA` 3 operation, but only AMD will support the :term:`FMA` 4
  operation. For the double precision data type :term:`FMA` 3 will be realized
  in three versions whose mnemonics are *VFMADD132PD*, *VFMADD213PD*
  and *VFMADD231PD*. The numbers 1, 2, and 3 indicate which registers will
  be multiplied and added \cite[Page 5]{Lomont2011}. The hardware realized
  :term:`FMA` 3 operation finally looks for example like this: *VFMADD132PD
  ymm0, ymm1, ymm2/m256*. The computation performed is *ymm0 = (ymm0 \cdot
  ymm2) + ymm1*. AMDs :term:`FMA` 4 has the form *VFMADDPD ymm1, ymm2, ymm3,
  ymm4/mem256*. This operation is non-destructive, that means, that no operand
  will be overwritten and remain available for further operations. *ymm1 =
  (ymm2 \cdot ymm3) + ymm4*. \cite[Chapter 14]{Intel2013} \cite[Chapter
  1]{AMD2013c}

\begin{figure}
\begin{bytefield}[bitwidth=0.1em,endianness=big,rightcurly=.]{256}
  \bitheader{0,128,255} \\ \begin{rightwordgroup}{YMM0}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM0}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM1}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM1}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM2}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM2}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM3}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM3}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM4}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM4}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM5}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM5}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM6}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM6}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM7}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM7}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM8}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM8}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM9}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM9}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM10}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM10}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM11}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM11}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM12}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM12}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM13}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM13}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM14}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM14}
  \end{rightwordgroup} \\ \begin{rightwordgroup}{YMM15}
    \bitbox{128}{} & \colorbitbox{Gray}{128}{XMM15}
  \end{rightwordgroup} \\
\end{bytefield} \caption{SSE registers.} \label{fig:SSE registers} \end{figure}

At the time of writing AMD64 already offers the :term:`FMA` 3 operation
in processors based upon the microarchitectures "Bulldozer" and
"Piledriver". The latter one supports the :term:`FMA` 4 operation as well
\cite[Page 2]{Hollingsworth2012}. The :term:`AVX` instruction set, including
:term:`FMA` 3, will be part of Intels fourth generation Core\texttrademark\
processors with the code name "Haswell" \cite[Page 1,4]{Meghana2013}.

With this background of floating-point instruction sets it is possible
to determine the availability of :term:`FMA` on a specific system. The
*CPUID* operation is very helpful to get information about AMD or Intel
processors. Calling *CPUID* returns a bit pattern that contains all
information about the :term:`CPU` features. The meanings of the individual bits
are described in \cite[Appendix E]{AMD2013b} for AMD or for Intel in \cite[Page
219-246]{Intel2013a}. Additionally Intel provides an assembler pseudo code
\cite[Chapter 14.5.3]{Intel2013} to check the availability of :term:`FMA`. A
system test program "SystemProperties",
attached to this thesis, contains a modified version of that assembler code.



.. _sec-Software and compiler support:

Software and compiler support
=============================

The compiler used in this project is the :term:`GCC` [#f7]_ , a free software
compiler, and it's C++ front end G++. As the manual suggest, the compiler
names :term:`GCC` and G++ can be used interchangeably for C++ source file
input \cite[Chapter 1]{Stallman2013}. :term:`GCC` links C programs against the
:term:`GLIBC` [#f8]_ [#f9]_ 
\cite[Chapter 3.17.13]{Stallman2013} and C++ programs are linked
against the :term:`LIBSTDC++` [#f10]_ . Using
:term:`GCC` guarantees easy reproducibility of this work. It follows a discussion
which options have to be passed to the :term:`GCC` compiler to fulfill the
following three properties:

* compliance with the standards :term:`C11` or :term:`C++11` and
  :term:`IEEE 754-2008`
* optimized code
* enabling the :term:`FMA` operation by hardware

To fulfill the first
property :term:`GCC` offers the options *-std=c11* or *-std=c++11*
respectively and *-pedantic* \cite[Chapter 2]{Stallman2013}. But :term:`GCC`
is not fully compliant to :term:`IEEE 754-2008`, :term:`C11`, and :term:`C++11`,
as online documents state [#f11]_ [#f12]_
\cite[Page 79]{Muller2010}. As long as this issue hasn't been fixed, it is up
to the programmer to carefully check the :term:`IEEE 754-2008` conformance. The
first case where one gets in touch with the broken implementation is the
contracted expression, described earlier for :term:`FMA`. The :term:`C11`
standard allows to control the usage of these expression with the
*FP\_ CONTRACT* pragma \cite[Chapter 7.12.2]{ISO/IEC2011a} \cite[Annex
F.6]{IEEE2008}. According to the :term:`GCC` manual this pragma is currently
not implemented. But contracting expressions is only enabled, if the
*-funsafe-math-optimizations* or *-ffast-math* options are used
\cite[Chapter 4.6]{Stallman2013}. There are two other switches to archive
standard compliance. The first one is *-fexcess-precision=standard*
ensuring that "double rounding" (see section :ref:`sec-Hardware realization`)
cannot occur as all casts and assignments are rounded to their semantic
type, regardless of being stored in for example an 80 bit register. This
option is enabled by default, if *-std* is used. The last one is
*-frounding-math* that disables the optimization assumption of the
default rounding mode. Doing this, the currently active rounding mode
is respected. One drawback is, that *-frounding-math* is marked as
experimental for the current version \cite[Chapter 3.10]{Stallman2013}.

When optimizing software, there is a trade-off between universality, means that
the resulting binaries run on a wide range of architectures and machines, and
the best possible optimization for one specific machine. As in this work the
new :term:`FMA` operation should be used, all optimization decisions are made to
primarily work best on the used test system (see Appendix :ref:`sec-Test system
information`). Now that the second and third properties are more hardware
depended, they can be examined together. :term:`GCC` offers the option *-O*
which allows to generically enable four levels of optimization (0-3). The
:term:`GCC` manual states, that even the highest optimization level *-O3*
does not invoke any options that are in conflict with the first targeted
property of standard compliance \cite[Chapter 3.10]{Stallman2013}. Thus it
is possible to use *-O3* safely. :term:`GCC` also offers many options
to enable or disable machine specific features and operations. To overcome
the big effort of checking all feature options for applicability, there is
the option *-march=native* that detects all available features of the
local machine and enables the feature options accordingly \cite[Chapter
3.17.16]{Stallman2013}. The for this thesis used machine is recognized
as *"bdver2"* for example. Because of this the :term:`FMA` options
*-mfma* and *-mfma4* are enabled too. This fulfills the third
desired target. All performed optimizations by the compiler can be inspected
using the option *-fopt-info* to ensure no undesired optimizations were
applied \cite[Chapter 3.10]{Stallman2013}.

Checking the processor feature bits, like it was done in Section
:ref:`sec-Hardware realization`, only gives the information, that :term:`FMA`
is available on a system. It does not guarantee, that the compiled program
makes use of that operation by hardware. To prove the latter, two steps
are suggested. The first one is to prove the specified property of single
rounding is held. The second one is to inspect the compiled assembler code to
find the :term:`FMA` instruction. A test case for the single rounding property
is given in the Appendix :ref:`sec-FMA test cases`. This
test case considers any rounding mode. An excerpt of the implementation of
the first FMA test case for :math:`exp_{a}
= exp_{b} = 0` is given in Listing :ref:`lst-Excerpts of the FMA test
case 1 implementation`. These test case programs could verify, that only
the hardware :term:`FMA` instruction results in the "result with FMA" from
Equation :eq:`eq-Mathematical description of FMA test case 1` and Equation
:eq:`eq-Mathematical description of FMA test case 2`.

A very short excerpt of the compiled program from Listing :ref:`lst-Excerpts of
the FMA test case 1 implementation` for *roundTowardNegative* is given
in Listing :ref:`lst-Excerpt from test 1 fma rd.s`. Indeed, the :term:`FMA` 4
operation (*vfmaddsd*) and an instance of the :term:`FMA` 3 operation
(*vfmadd231sd*) are used in the form of a scalar double-precision
operation, because of the "sd" suffix \cite[Page 544-552]{AMD2013c}. Even if
compiled with the highest optimization level *-O3*, it is not possible to
use packed double-precision operations, due to data dependencies in the program
of Listing :ref:`lst-Excerpts of the FMA test case 1 implementation`. But
for this test program this circumstance is of minor interest.



.. _sec-Performance:

Performance
===========

With being able to use the :term:`FMA` operation it is now interesting to
compare the performance to other basic operations. The benchmark program
simply repeats an operation on a given amount of data. To be more precise,
an outer loop increases the number of repetitions, in this case ten steps
from :math:`10^{9}` to :math:`10^{10}` repetitions. This is done to see how the operation
scales compared to others. The most important excerpts of the benchmark
program are in Listing :ref:`lst-Excerpt from benchmark fma.cpp`. A look
in the compiled routines (Listings :ref:`lst-Excerpt from benchmark fma
1.s`, :ref:`lst-Excerpt from benchmark add 1.s` and :ref:`lst-Excerpt from
benchmark mult 1.s`) reveals, that the repeated floating-point operation
is performed in a scalar and serial way, indicated by the "sd" suffix,
like described in Section :ref:`sec-Hardware realization`. Additionally
the code parallelization by a technique called "partial loop unrolling"
\cite[Chapters 3.4 and 8.2]{AMD2012} is taken into account by the benchmark
program. Basically this technique means nothing more than to repeat the code
for a distinct and independent memory location, e.g. *var[i]* and *var[i +
1]*. Independent means, that *var[i]* is not computed from *var[i + 1]*
in a preceding floating-point operation. Partial loop unrolling violates
the well-known *Don't Repeat Yourself* design principle, but it allows
the compiler to make relaxed assumptions about instruction reordering and
register usage. This allows to perform more floating-point operations per
clock cycle, thus the instruction-level parallelism increases \cite[Chapters
3.4 and 8.2]{AMD2012}. The compiled version of the parallelized benchmark
program is shown in Listing :ref:`lst-Excerpt from benchmark fma 4.s`. A
further step would be to make use of packed value ("pd") operations, but
later in the proposed algorithms these instructions cannot be used, due to
data dependencies and thus it is not considered in the benchmark program. The
results of the benchmark program for five levels of parallelism are shown
in Figure :ref:`fig-FMA performance compared to addition and multiplication`.

\begin{figure} \centering
\includegraphics[width=\textwidth]{pic/BenchmarkFma/benchmark_fma_add_mult}
\caption[FMA performance.]{:term:`FMA` performance compared to addition
and multiplication.} \label{fig:FMA performance compared to addition and
multiplication} \end{figure}

The results of this benchmark program verify, that the pure :term:`FMA`
operation is as fast as a simple addition and multiplication for AMD
"Piledriver" :term:`CPU` s. The same result was taken by another benchmark
as well \cite[Page 60]{Fog2013}. The reason for the equal results is the
usage of the  same floating-point multiply/add subunit for all of these
operations \cite[Pages 49 and 60]{Fog2013}. Thus :term:`FMA` can be used as
a hardware implemented operation without having any performance penalties.

.. rubric:: Footnotes

.. [#f1] Advanced Micro Devices, Inc.
.. [#f2] Intel Corporation
.. [#f3] This does not necessarily mean a single :term:`CPU` clock cycle.
.. [#f4] Also known as AMD x86-64.
.. [#f5] Instruction set introduced by AMD.
.. [#f6] Terms "legacy :term:`SSE` " and "extended :term:`SSE` " are adopted from \cite[Chapter 4.1.2]{AMD2013} for this thesis.
.. [#f7] http://gcc.gnu.org/
.. [#f8] https://www.gnu.org/software/libc/
.. [#f9] Note that Ubuntu 13.10 uses http://www.eglibc.org/, which is based upon :term:`GLIBC` .
.. [#f10] http://gcc.gnu.org/libstdc++/
.. [#f11] http://gcc.gnu.org/c99status.html
.. [#f12] http://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html

