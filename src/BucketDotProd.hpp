#if !defined(BUCKET_DOT_PROD_HPP_)
#define BUCKET_DOT_PROD_HPP_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

// NDEBUG definied turns off assertions
#define NDEBUG

// constants for significant partitioning
#define BD_OFFSET     2
#define BD_PART1     15
#define BD_GUARD      2
#define BD_SHIFT     18
#define BD_PART3     16
#define BD_PRECISION 53
#define BD_PART1_OVERFLOW 11

// constants describing the bucket array
#define BD_NUMBER_OF_UNDERFLOW_BUCKETS 5
#define BD_NUMBER_OF_NORMAL_BUCKETS 112	// roundTowardNegative((2^11) / SHIFT) - 1
#define BD_NUMBER_OF_OVERFLOW_BUCKETS 2
#define BD_NUMBER_OF_BUCKETS (BD_NUMBER_OF_UNDERFLOW_BUCKETS + \
  BD_NUMBER_OF_NORMAL_BUCKETS + BD_NUMBER_OF_OVERFLOW_BUCKETS)

#define BD_ACCUMULATION_RESERVE 32766		// (2^PART1 - 2) * PARALLEL / 2
#define BD_ACCUMULATION_RESERVE_OVERFLOW 2046		// (2^PART1_OVERFLOW - 2)  * PARALLEL / 2

// constants for division by 18 substitution. For x in [0 2048].
#define BD_DIV_18_MULTIPLIER 1821
#define BD_DIV_18_SHIFT 15

/**
 * Computes an accurate inner product.
 */
class BucketDotProd
{
private:
  // bucket arrays
  double *a1;
  double *a2;

  // bucket initialization masks
  double *mask;

protected:
  void initialize_buckets();

public:
  // Data type for analyzing an IEEE 754 double precision floating-point value.
  typedef struct
  {
    unsigned mantissa_low:32;
    unsigned mantissa_high:20;
    unsigned exponent:11;
    unsigned sign:1;
  } ieee754_double;

  /**
   * Constructor
   */
  BucketDotProd ();

  /**
   * Destructor
   */
  ~BucketDotProd ();

  /**
   * Sums up all elements of abritary condition number with fixed memory usage.
   *
   * @param x first input vector
   * @param y second input vector
   * @param n length of the input vectors
   * @return correctly rounded inner product of the vectors
   */
  double dprod (double *x, double *y, int n);
};

#endif

