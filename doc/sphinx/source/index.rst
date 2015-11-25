.. Accurate Algorithms documentation master file.

*******************
Accurate Algorithms
*******************

The inner product is one of the most elementary algebraic operations and the
basis for a large number of numerical applications and computations that are
performed using binary floating-point arithmetic on computers. Depending on
the condition of the input data, straight forward implementations of the inner
product are very inaccurate and of limited use for verified computations. To
overcome this issue, many algorithms have been developed in the past with
different strengths and weaknesses. This project introduces new algorithms
for summation and inner product computation, that make use of the :term:`FMA`
instruction, which will be part of upcoming state of the art computer
instruction sets. The proposed algorithms scale well for vector lengths of
about :math:`10^{3}` elements and more.

Contents:

.. toctree::
   :maxdepth: 2

   ch01intro
   ch02ieee754
   ch03fma
   ch04summation
   ch05dotprod
   ch06conclusion
   ch07glossary
   ch08references
   ch09appendix

