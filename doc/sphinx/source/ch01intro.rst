.. _ch-introduction:

************
Introduction
************

This project deals with the software realization of one of the most elementary
algebraic operation, the inner product. The first objective is to compute the
inner product as accurate as possible without violating the second objective to
do it as efficient as possible for middle and large dimension vector lengths.
Hereby middle dimension describes vectors with about :math:`10^{3}` elements and
accordingly large dimension vectors with more than :math:`10^{6}` elements.
[#f1]_

The inner product is the basis for a large number of numerical applications and
computations that are performed using binary floating-point arithmetic on
computers. Therefore in Chapter :ref:`ch-ieee754` this floating-point
arithmetic, specified by the IEEE, is introduced. One part of this specification
are the rounding modes, that are examined in detail. Furthermore methods are
presented to efficiently extract the exponent part of a :term:`binary64`.
Another part of the IEEE specification is the :term:`FMA` instruction. This
instruction is one of the main reasons for this project, as it becomes available
as hardware realization for upcoming generations of consumer processors. The
:term:`FMA` instruction itself and the usage from the programming languages C
and C++ are analyzed comprehensively in Chapter :ref:`ch-fma`.

Computing accurate inner products is strongly connected to computing accurate
sums, which are treated in Chapter :ref:`ch-summation`. In this chapter some
previous approaches and basic concepts are presented, followed by the
introduction of the new algorithm *BucketSum* for accurate summation. In Chapter
:ref:`ch-dotprod` many of the summation approaches are advanced in order to
compute accurate inner products. Especially *BucketSum* is extended to
*BucketDotProd* and *FMAWrapperDotProd*. Both algorithms exploit the features of
the :term:`FMA` instruction. In this chapter it is shown, that other algorithms
benefit from the existence of a hardware version of :term:`FMA` as well. All
presented algorithms are therefore tested numerically using a benchmark program
for summation and inner products in the corresponding chapters. The test system
for the benchmark programs is described in Appendix :ref:`sec-Test system
information`. Finally, the results are concluded in Chapter :ref:`ch-conclusion`
with a perspective for the future.

.. rubric:: Footnotes

.. [#f1] Note that a vector with :math:`10^{9}` :term:`binary64` elements would
         almost completely fill the whole 8 GB of main memory of a state of the
         art computer system.

