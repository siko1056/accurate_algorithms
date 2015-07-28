#if !defined(BUCKET_SUM_HPP_)
#define BUCKET_SUM_HPP_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

// NDEBUG definied turns off assertions
#define NDEBUG

// constants for significant partitioning
#define OFFSET     2
#define PART1     15
#define GUARD      2
#define SHIFT     18
#define PART3     16
#define PRECISION 53
#define PART1_OVERFLOW 11

// constants describing the bucket array
#define NUMBER_OF_UNDERFLOW_BUCKETS 2
#define NUMBER_OF_NORMAL_BUCKETS 112	// roundTowardNegative((2^11) / SHIFT) - 1
#define NUMBER_OF_OVERFLOW_BUCKETS 2
#define NUMBER_OF_BUCKETS (NUMBER_OF_UNDERFLOW_BUCKETS + \
  NUMBER_OF_NORMAL_BUCKETS + NUMBER_OF_OVERFLOW_BUCKETS)

#define ACCUMULATION_RESERVE 32766 * 2		// (2^PART1 - 2) * PARALLEL
#define ACCUMULATION_RESERVE_OVERFLOW 2046 * 2		// (2^PART1_OVERFLOW - 2)  * PARALLEL

// constants for division by 18 substitution. For x in [0 2048].
#define DIV_18_MULTIPLIER 1821
#define DIV_18_SHIFT 15

/**
 * Computes an accurate sum.
 */
class BucketSum
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
  BucketSum ();

  /**
   * Destructor
   */
  ~BucketSum ();

  /**
   * Sums up all elements of abritary condition number with fixed memory usage.
   *
   * @param input array of addends
   * @param n length of the array of addends
   * @return correctly rounded sum of the addends
   */
  double sum (double *input, int n);
};

#endif

