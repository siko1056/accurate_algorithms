.. _ch-dotprod:

**********************
Accurate inner product
**********************

This chapter is the combination of all previous chapters. With the :term:`FMA`
instruction and an efficient and fast summation algorithm, one is able to
create efficient inner product algorithms as well. After a short overview
about previous approaches, three algorithms using :term:`FMA` will be introduced.



Previous work
=============

Like the Recursive summation (Algorithm :ref:`alg-RecursiveSummation`),
there also exists a simple straight forward implementation of the
inner product, namely Algorithm :ref:`alg-RecursiveDotProduct` (see
\cite[Algorithm 6.2]{Muller2010}). A condition number for the inner product
(Equation :eq:`eq-Dot product condition number`) is defined in \cite[Chapter
6.1.2]{Muller2010} as well:

.. math::
  :label: eq-Dot product condition number

   R_{\text{dot product}} =
   \dfrac{2 \cdot \displaystyle\sum_{i = 1}^{n} \vert a_{i} \cdot b_{i}\vert}
   {\left\vert\displaystyle\sum_{i = 1}^{n} a_{i}\cdot b_{i}\right\vert}


.. code-block:: octave
   :caption: Recursive inner product
   :name: alg-RecursiveDotProduct
   :linenos:

   function RecursiveDotProduct(a, b, N)
     s = fl(a_{1} \cdot b_{1})
     for(i = 2 to N)
       s = fl(s + fl(a_{i} \cdot b_{i}))
     end for
     return s
   end function

Similar to the definition of the error-free transformation for the sum,
there also exists a definition for the product in \cite[Chapter 3]{Rump2005}:

.. math::
   :label: eq-Error-free transformation product

   a \cdot b = x + y \quad\text{and}\quad x = fl(a \cdot b)

This means the
product is transformed into a sum. By knowing this, it becomes obvious why the
task of finding an efficient inner product algorithm is strongly connected
with the task of accurate summation. If the factors are floating-point
numbers :math:`a, b \in \mathbb{F}`, then :math:`x, y \in \mathbb{F}` holds, like for the
error-free transformation of the sum, too. :math:`x \in \mathbb{F}` follows from
the definition in Equation :eq:`eq-Error-free transformation product`. An
example for :math:`y \in \mathbb{F}` for the special case of :term:`binary64` is
given in Equation :eq:`eq-Error-free transformation product example`. If
*a* and *b* have the biggest possible significant (all bits set to "1"),
then their product cannot exceed 106 bits. This fits exactly into two 53
bit precisions of a :term:`binary64`.

.. math::
  :label: eq-Error-free transformation product example

   \begin{aligned}
   a &= \left(2^{53} - 2^{0}\right) \cdot 2^{exp_{a}} \nonumber \\ b &=
   \left(2^{53} - 2^{0}\right) \cdot 2^{exp_{b}} \nonumber \\ c &= a \cdot b =
   \left(2^{106} - 2^{54} + 2^{0}\right)
     \cdot 2^{exp_{a} + exp_{b}} \\
   x &= fl(c) = \left(2^{106} - 2^{54}\right)
     \cdot 2^{exp_{a} + exp_{b}} \nonumber \\
   y &= a \cdot b - fl(c) = \left(2^{0}\right)
     \cdot 2^{exp_{a} + exp_{b}} \nonumber
   \end{aligned}

Without the :term:`FMA` instruction, there exists the algorithm
*TwoProduct*, that is able to perform this error-free product
transformation by using 17 :term:`FLOP` s (see \cite[Chapter 3]{Rump2005}
for details). Having a system with a hardware implemented :term:`FMA`
instruction, the whole effort can be reduced to *TwoProductFMA*
(Algorithm :ref:`alg-TwoProductFMA`). This algorithm is also described in
\cite[Algorithm 3.5]{Rump2005} and requires only two :term:`FLOP` s.

.. code-block:: octave
   :caption: Error-free transformation TwoProductFMA
   :name: alg-TwoProductFMA
   :linenos:

   function TwoProductFMA(a, b)
     x = fl(a \cdot b)
     y = FMA(a, b, -x)
     return (x, y)
   end function

For the inner product the idea of error-free transformation can
also be extended from two to *N* operands with *Dot2* (Algorithm
:ref:`alg-Dot2`). *Dot2* computes :math:`\sum_{i = 1}^{N} x_{i} \cdot y_{i}`
as if twice the working precision was used \cite[Chapter 5]{Rump2005}. In
that paper the idea has been extended to algorithm *DotK*, which can
evaluate the inner product, as if computed with K-fold working precision. A
slightly modified version of *Dot2* will be presented in the next chapter.

.. code-block:: octave
   :caption: Inner product in twice the working precision Dot2
   :name: alg-Dot2
   :linenos:

   function Dot2(x, y, N)
     [p, s] = TwoProduct(x_{1}, y_{1})
     for(i=2 to N)
       [h, r] = TwoProduct(x_{i}, y_{i})
       [p, q] = TwoSum(p, h)
       s = fl(s + fl(q + r))
     end for
     return fl(p + s)
   end function



Algorithms based upon TwoProductFMA
===================================

The first algorithm that is tested in the following benchmark (Chapter
:ref:`sec-Benchmark DotProd`) is *Dot2* (Algorithm :ref:`alg-Dot2`) with
all occurrences of *TwoProduct* replaced by *TwoProductFMA*. This
algorithm will be called *Dot2FMA* in the following. This modification
is already described in \cite[Chapter 5]{Rump2005}.

Another trivial idea is not to modify the existing summation algorithms
of Chapter :ref:`ch-summation`. Instead a preprocessing
of the input vectors is done with *TwoProductFMA* (Algorithm
:ref:`alg-TwoProductFMA`). This approach will be called *FMAWrapperDotProd*
and is described in Algorithm :ref:`alg-FMAWrapperDotProd` in combination with
BucketSum. FMAWrapperDotProd has two major flaws. The first one is connected
with the data preprocessing. The implementer has to decide whether the
method should preserve the input vectors or not. In the first case the memory
requirement increases by twice the size of the input vector length, in the
latter case the original input vectors are lost. The second flaw is related to
the usage of the summation algorithm in Algorithm :ref:`alg-FMAWrapperDotProd`
lines 7-8. These lines create an intermediate rounding, that dependent on
the resulting vectors can return a not correctly rounded result. A solution
to this problem would be an interface method, that allows to accumulate
a vector of a certain size, and a second one to make a final sum up to a
correctly rounded sum. Such an interface is for example available in the
implementation of *OnlineExactSum* \cite{Zhu:2010:A9O:1824801.1824815}.

.. code-block:: octave
   :caption: FMAWrapperDotProd
   :name: alg-FMAWrapperDotProd
   :linenos:

   \Require *N*, number of addends
   \Require *x*, first array of length *N*
   \Require *y*, second array of length *N*
   \Ensure $s$, computed inner product $\sum_{i = 1}^{N} x_{i} \cdot y_{i}$
   function FMAWrapperDotProd(x, y, N)
     for(i = 1 to N) % In-place array preprocessing
       t = fl(x_{i} \cdot y_{i}) % Destructive TwoProductFMA
       y_{i} = FMA(x_{i}, y_{i}, -t)
       x_{i} = t
     end for
     s = BucketSum(x, N) % Any summation algorithm possible
     s = fl(s + BucketSum(y, N))
     return s
   end function

Finally a modified version of *BucketSum* (Algorithm
:ref:`alg-BucketSum`) is presented, namely *BucketDotProd*
(Algorithm :ref:`alg-BucketDotProd`). *BucketDotProd* is identical to
*BucketSum*, except for the lines 8-13, where *TwoProductFMA*
comes into play. Assume the product to accumulate is $a \cdot b$, therefore
$x = fl(a \cdot b)$ and $y = FMA(a, b, -a \cdot b)$. It was already shown,
that if *x* has to be added to bucket *i* and its error to bucket *i - 2*,
no significant bit is lost (`Theorem 1`_). In order
to avoid the expensive three :term:`FLOP` s for the exponent extraction of
*y*, one can make use of the *shift = 18* property for the :term:`binary64`
realization. In that case *y* will always fall in the exponent range of
bucket *i - 3*. According to `Theorem 1`_ the error of *y*
has to be added to bucket *i - 5*.

\begin{figure} \centering
\includegraphics[width=\textwidth]{pic/accumulation_bucket_dot_prod}
\caption{Visualization of BucketDotProds accumulation.} \label{fig:accumulation
bucket dot prod} \end{figure}

.. code-block:: octave
   :caption: BucketDotProd
   :name: alg-BucketDotProd
   :linenos:

   \Require *N*, number of addends
   \Require *x*, first array of length *N*
   \Require *y*, second array of length *N*
   \Ensure $s$, correctly rounded inner product $\sum_{i = 1}^{N} x_{i} * y_{i}$
   function BucketDotProd(x, y, N)
     % $a_{1}$ and $a_{2}$ are underflow and
     % $a_{M - 1}$ and $a_{M}$ are overflow buckets
     \State create arrays of $M$ buckets *a* % $a_{2 \dots M - 2}$ cover $SHIFT$ exponents
     \State create arrays of $M$ bucket masks $mask$ % $mask_{1}$ is set to $0$, $mask_{M}$ to NaN
     \State initialize $a_{i}$ with $mask_{i}$
     sum = 0
     for(i = 1 to N)
       (v, w) = TwoProductFMA(x_{i}, y_{i})
       pos = \lceil exp(v) / SHIFT \rceil + 2 % $exp()$ extracts biased exponent
       (a_{pos}, e_{1}) = FastTwoSum(a_{pos}, v)
       (a_{pos - 3}, e_{2}) = FastTwoSum(a_{pos - 3}, w)
       a_{pos - 2} = fl(a_{pos - 2} + e_{1})
       a_{pos - 5} = fl(a_{pos - 5} + e_{2})
       if((i \mod C1) == 0)                  % $C1 =$ capacity of normal buckets
         for(j = 1 to (M - 2))                          % Tidy up normal buckets
           r = fl(fl(mask_{j + 1} + fl(a_{j} - mask_{j})) - mask_{j + 1})
           a_{j + 1} = fl(a_{j + 1} + r)
           a_{j} = fl(a_{j} - r)
         end for
       end if
       if((i \mod C2) == 0)                % $C1 =$ capacity of overflow buckets
         sum = fl(sum + fl(a_{M - 1} - mask_{M - 1}))         % Tidy up overflow
         a_{M - 1} = mask_{M - 1}
       end if
     end for
     for(i = 1 to (M - 1)) % remove masks
       a_{i} = a_{i} - mask_{i}
     end for
     return ModifiedKahanSum(sum, a_{M-1 \text{ downto } 1}, M-1)
   end function

This chapter shows, that with moderate effort nearly each summation algorithm
can be modified to handle the task of inner product computation as well. In
the following numerical tests show the properties of these three algorithms
in a benchmark program.



.. _sec-benchmark dotprod:

Benchmark
=========

For the benchmark of inner product the five algorithms of Table
:ref:`tbl-Comparison of inner product algorithms` are compared. All algorithms
were implemented as part of this Master's Thesis. Only for the implementation
of *Dot2* and *Dot2FMA* some sub-functions of \cite{Lathus2012}
were used. Like for the summation benchmark the C-XSC toolbox has been used
to verify the correctness of the computed inner products.

.. list-table:: Comparison of inner product algorithms for input data length *N*
   :header-rows: 1
   :name: tbl-Comparison of inner product algorithms

   * - Algorithm
     - :term:`FLOP` s
     - Run-time
     - Space
   * - *Recursive Inner Product* (Algorithm :ref:`alg-RecursiveDotProduct`)
     - :math:`2N-1`
     - 1
     - :math:`\mathcal{O}(1)`
   * - *Dot2* (Algorithm :ref:`alg-Dot2`)
     - :math:`25N-7`
     - 5-6
     - :math:`\mathcal{O}(1)`
   * - *Dot2FMA* (Algorithm :ref:`alg-Dot2`)
     - :math:`6N-3`
     - 3-4
     - :math:`\mathcal{O}(1)`
   * - *FMAWrapperDotProd* (Algorithm :ref:`alg-FMAWrapperDotProd`)
     - :math:`16N`
     - 4-6*
     - :math:`\mathcal{O}(N)`
   * - *BucketDotProd* (Algorithm :ref:`alg-BucketDotProd`)
     - :math:`13N`
     - 3-4*
     - :math:`\mathcal{O}(1)`

The asterisk "*" in :ref:`tbl-Comparison of inner product algorithms` indicates
the use of instruction-level parallelism.
For the inner product benchmark four kinds of test data are used. **Data 1**
are two random, positive floating-point vectors of length *N*, all with
an exponent of :math:`2^{0}`. **Data 2** is well-conditioned like Data 1,
but each of the two input vectors has a random distributed exponent range
between :math:`2^{0}` and :math:`2^{400}`. **Data 3** is ill-conditioned with a random
distributed exponent between :math:`2^{-400}` and :math:`2^{400}`. Finally
**Data 4** is ill-conditioned, with a real inner product of exactly zero. The
time measuring and the determination of the middle and large dimension
data lengths happens in the same way as in Chapter :ref:`sec-Benchmark
summation`. Especially the assumptions for the data length determination
allows the creation of two arrays, without exceeding the available main memory.

.. figure:: _static/result_inner_product_middle_dimension_data_1.svg
   :alt: Well-conditioned, equal exponent.
   :name: fig-DotProd Middle Data 1
   :align: center

   Well-conditioned, equal exponent.

.. figure:: _static/result_inner_product_middle_dimension_data_2.svg
   :alt: Well-conditioned, large exponent range.
   :name: fig-DotProd Middle Data 2
   :align: center

   Well-conditioned, large exponent range.

.. figure:: _static/result_inner_product_middle_dimension_data_3.svg
   :alt: Ill-conditioned, large exponent range.
   :name: fig-DotProd Middle Data 3
   :align: center

   Ill-conditioned, large exponent range.

.. figure:: _static/result_inner_product_middle_dimension_data_4.svg
   :alt: Ill-conditioned, zero result.
   :name: fig-DotProd Middle Data 4
   :align: center



.. figure:: _static/result_inner_product_large_dimension_data_1.svg
   :alt: Well-conditioned, equal exponent.
   :name: fig-DotProd Large Data 1
   :align: center

   Well-conditioned, equal exponent.

.. figure:: _static/result_inner_product_large_dimension_data_2.svg
   :alt: Well-conditioned, large exponent range.
   :name: fig-DotProd Large Data 2
   :align: center

   Well-conditioned, large exponent range.

.. figure:: _static/result_inner_product_large_dimension_data_3.svg
   :alt: Ill-conditioned, large exponent range.
   :name: fig-DotProd Large Data 3
   :align: center

   Ill-conditioned, large exponent range.

.. figure:: _static/result_inner_product_large_dimension_data_4.svg
   :alt: Ill-conditioned, zero result.
   :name: fig-DotProd Large Data 4
   :align: center

   Ill-conditioned, zero result.

The results of the inner product benchmark, shown in Figures above,
verify a linear scaling
of the algorithms in Table :ref:`tbl-Comparison of inner product algorithms` for
data lengths in each, middle and large dimension. Another observation is, that
the type of the data does not really affect the runtime of the algorithms. In
any case *BucketDotProd* is the fastest algorithm and only by factor
2-3 slower than the Recursive Inner Product. *FMAWrapperDotProd*,
that only preprocesses the input vectors, is already by factor 4-6 slower
than the Recursive Inner Product. The reason for this seems to be, that all
input vectors have to be processed  completely twice. Another improvement is
observable if *Dot2* is used in combination with *TwoProductFMA* in
*Dot2FMA*. The execution time nearly halves, if a hardware implemented
:term:`FMA` instruction is available on the system. The result accuracy is
again checked by an *assert()*-statement against the result of the
C-XSC toolbox, like it was done for the summation benchmark. Therefore the
results of *BucketDotProd* are claimed to be correctly rounded. This
check cannot be applied for *FMAWrapperDotProd*, because of the in the
previous chapter discussed implementation drawback.

