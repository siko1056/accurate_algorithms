# Accurate Algorithms
This repository contains a collection of algorithms for the accurate computation of sums and dot products, making use of the [Fused Multiply-Add (FMA)](https://en.wikipedia.org/wiki/Multiply%E2%80%93accumulate_operation#Fused_multiply.E2.80.93add) operation and error-free transformations.

## Motivation

* Binary floating-point arithmetic has its limitations. For example `1 + M + 2*M - 3*M` is inaccurate for `M` &ge; 2<sup>52</sup> &asymp; 4.5*10<sup>15</sup>
* Only very basic operations (`+`, `-`, `*`, `/`, &radic;, `FMA`) are hardware implemented, fast, and accurate.  
* The dot product is the basis for many numerical applications, thus should be fast and accurate as well.
* New possibilities for dot product algorithms with **FMA**.
  * Specified in the floating-point standard ([IEEE 754-2008](https://en.wikipedia.org/wiki/IEEE_floating_point)) `FMA(a, b, c) := fl((a * b) + c)`, where `fl` means correctly rounded. 
  * Part of the [C/C++ standard library](https://en.wikipedia.org/wiki/C%2B%2B11)
```
c = std::fma (a, b, c);
```
  * Hardware realization on newer computer achitectures via the [x86-64](https://en.wikipedia.org/wiki/X86-64) instruction set, thus as fast as for example a multiplication. Packed value versions (parallel computations) of the **FMA** instruction also exist.
```
vfmadd231sd %xmm1, %xmm2, %xmm3
```
  * Software emulation, if not supported by hardware.

## Error-free transformation

TODO

## Similar approaches

* [The GNU MPFR Library](http://www.mpfr.org/)
* [C-XSC Library](http://www2.math.uni-wuppertal.de/wrswt/xsc/cxsc.html)

## Selected references

* *Takeshi Ogita, Siegfried M. Rump, and Shin'ichi Oishi.* 2005. **Accurate Sum and Dot Product.** SIAM J. Sci. Comput. 26, 6 (June 2005), 1955-1988. http://dx.doi.org/10.1137/030601818
* *Yong-Kang Zhu and Wayne B. Hayes.* 2010. **Algorithm 908: Online Exact Summation of Floating-Point Streams.** ACM Trans. Math. Softw. 37, 3, Article 37 (September 2010), 13 pages. http://dx.doi.org/10.1145/1824801.1824815
