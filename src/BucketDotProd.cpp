#include "BucketDotProd.hpp"

#if !defined(NDEBUG)
/**
 * Helper function to detect missalignment of the buckets.
 */
inline void
verify_bucket_positions (double* buckets)
{
  for (int i = 0; i < (BD_NUMBER_OF_UNDERFLOW_BUCKETS - 1); i++)
    {
      assert (((BucketDotProd::ieee754_double *) & buckets[i])->exponent <=
              (std::numeric_limits<double>::min_exponent +
               (std::numeric_limits<double>::max_exponent - 1)));
    }
  for (int i = (BD_NUMBER_OF_UNDERFLOW_BUCKETS - 1);
       i < (BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS); i++)
    {
      assert (((BucketDotProd::ieee754_double *) & buckets[i])->exponent ==
              ((std::numeric_limits<double>::min_exponent - 1) +
               ((i - (BD_NUMBER_OF_UNDERFLOW_BUCKETS - 2)) * BD_SHIFT) - 1 +
               (std::numeric_limits<double>::max_exponent - 1)));
    }

  assert (((BucketDotProd::ieee754_double *) &
           buckets[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS])->
          exponent == 2 * (std::numeric_limits<double>::max_exponent - 1));
  // buckets[NUMBER_OF_BUCKETS - 1] is always NaN
}
#endif

/**
 * Constructor
 */
BucketDotProd::BucketDotProd ()
{
  // allocate bucket memory
  a1 = new double[BD_NUMBER_OF_BUCKETS];
  a2 = new double[BD_NUMBER_OF_BUCKETS];

  // create bucket masks
  mask = new double[BD_NUMBER_OF_BUCKETS];
  for (int i = 0; i < (BD_NUMBER_OF_UNDERFLOW_BUCKETS - 1); i++)
    mask[i] = 0.0;
  mask[BD_NUMBER_OF_UNDERFLOW_BUCKETS - 1] =
    1.5 * std::pow (2.0, (std::numeric_limits<double>::min_exponent - 1) +
                    (BD_SHIFT - 1));
  for (int i = BD_NUMBER_OF_UNDERFLOW_BUCKETS;
       i < (BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS); i++)
    mask[i] = mask[i - 1] * std::pow (2.0, BD_SHIFT);
  mask[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS] =
    1.5 * std::pow (2.0, (std::numeric_limits<double>::max_exponent - 1));
  mask[BD_NUMBER_OF_BUCKETS - 1] = std::numeric_limits<double>::quiet_NaN();

  // prepare buckets
  initialize_buckets ();
}

/**
 * Destructor
 */
BucketDotProd::~BucketDotProd ()
{
  delete [] a1;
  delete [] a2;
  delete [] mask;
}

void
BucketDotProd::initialize_buckets ()
{
  for (int i = 0; i < BD_NUMBER_OF_BUCKETS; i++)
    a1[i] = mask[i];
  for (int i = 0; i < BD_NUMBER_OF_BUCKETS; i++)
    a2[i] = -mask[i];

#if !defined(NDEBUG)
  verify_bucket_positions (a1);
  verify_bucket_positions (a2);
#endif
}

/**
 * Sums up all elements of abritary condition number with fixed memory usage.
 *
 * @param x first input vector
 * @param y second input vector
 * @param n length of the input vectors
 * @return correctly rounded inner product of the vectors
 */
double
BucketDotProd::dprod (double *x, double *y, int n)
{
  double sum = 0.0;

  // check input, main loop preparation requires more than one addend
  if (n < 1)
    return sum;
  else if (n == 1)
    return x[0] * y[0];

  // if n is odd, treat first elements seperately
  if (n & 1)
    {
      // preparation
      double addend1 = x[0] * y[0];
      double addend2 = std::fma(x[0], y[0], -addend1);
      unsigned pos = ((ieee754_double *) (&addend1))->exponent;
      pos = ((pos * BD_DIV_18_MULTIPLIER) >> BD_DIV_18_SHIFT);

      // addend1
      double t1 = a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS] + addend1;
      a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 2] +=
        (a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS] - t1) + addend1;
      a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS] = t1;

      // addend2
      double t2 = a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] + addend2;
      a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 5] +=
        (a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] - t2) + addend2;
      a1[pos + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] = t2;

      x++;
      y++;
      n--;
    }

  // prepare accumulation of remaining addends
  int ov_counter = 1;  // counter for overflow tidy up

  double addend1_1 = x[0] * y[0];
  double addend1_2 = std::fma(x[0], y[0], -addend1_1);
  double addend2_1 = x[1] * y[1];
  double addend2_2 = std::fma(x[1], y[1], -addend2_1);
  unsigned pos1 = ((ieee754_double *) (&addend1_1))->exponent;
  unsigned pos2 = ((ieee754_double *) (&addend2_1))->exponent;
  pos1 = ((pos1 * BD_DIV_18_MULTIPLIER) >> BD_DIV_18_SHIFT);
  pos2 = ((pos2 * BD_DIV_18_MULTIPLIER) >> BD_DIV_18_SHIFT);

  // main loop for accumulation
  while (1)
    {
      // number of addend till end-2 of input array or next tidy-up
      int limit = std::min (BD_ACCUMULATION_RESERVE_OVERFLOW, n - 2);

      // fast loop over even numbers of addends -> add into 2 accumulators
      for (int i = 0; i < limit; i += 2)
        {
          // two parallel FastTwoSum and extraction of the next two exponents
          double t1_1 = a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] + addend1_1;
          double t1_2 = a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] + addend1_2;
          double t2_1 = a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] + addend2_1;
          double t2_2 = a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] + addend2_2;

          // interleaved preparation for next loop
          double addend1_1_new = x[(i + 2)] * y[(i + 2)];
          double addend1_2_new =
            std::fma(x[(i + 2)], y[(i + 2)], -addend1_1_new);
          double addend2_1_new = x[(i + 3)] * y[(i + 3)];
          double addend2_2_new =
            std::fma(x[(i + 3)], y[(i + 3)], -addend2_1_new);
          unsigned pos1_new = ((ieee754_double *) (&addend1_1_new))->exponent;
          unsigned pos2_new = ((ieee754_double *) (&addend2_1_new))->exponent;
          pos1_new = ((pos1_new * BD_DIV_18_MULTIPLIER) >> BD_DIV_18_SHIFT);
          pos2_new = ((pos2_new * BD_DIV_18_MULTIPLIER) >> BD_DIV_18_SHIFT);

          a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 2] +=
            (a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] - t1_1) + addend1_1;
          a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 5] +=
            (a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] - t1_2) + addend1_2;

          a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 2] +=
            (a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] - t2_1) + addend2_1;
          a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 5] +=
            (a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] - t2_2) + addend2_2;

          a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] = t1_1;
          a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] = t2_1;
          a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] = t1_2;
          a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] = t2_2;

          // update for next loop
          addend1_1 = addend1_1_new;
          addend1_2 = addend1_2_new;
          addend2_1 = addend2_1_new;
          addend2_2 = addend2_2_new;
          pos1 = pos1_new;
          pos2 = pos2_new;
        }

      // leave loop if no input entries left
      if (limit == (n - 2))
        break;

      // update input position and number of remaining addends
      x += limit;
      y += limit;
      n -= limit;
      ov_counter++;

      // tidy up accumulation array
      if (ov_counter *  BD_ACCUMULATION_RESERVE_OVERFLOW > BD_ACCUMULATION_RESERVE)
        {
          for (int i = 0; i < (BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS);
               i++)
            {
              double t1 = (a1[i] + a2[i]);
              double t2 = a1[i + 1] + t1;
              a1[i] = mask[i] + ((a1[i + 1] - t2) + t1);
              a1[i + 1] = t2;
              a2[i] = -mask[i];
            }
          ov_counter = 1;
        }

      // tidy up overflow range
      sum +=
        (a1[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS] +
         a2[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS]);
      a1[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS] =
        mask[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS];
      a2[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS] =
        -mask[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS];
    }

  // treat last two addends seperately to avoid array bound exceeding
  // two parallel FastTwoSum
  double t1_1 = a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] + addend1_1;
  double t1_2 = a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] + addend1_2;
  double t2_1 = a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] + addend2_1;
  double t2_2 = a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] + addend2_2;
  a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 2] +=
    (a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] - t1_1) + addend1_1;
  a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 5] +=
    (a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] - t1_2) + addend1_2;
  a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 2] +=
    (a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] - t2_1) + addend2_1;
  a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 5] +=
    (a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] - t2_2) + addend2_2;
  a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] = t1_1;
  a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS] = t2_1;
  a1[pos1 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] = t1_2;
  a2[pos2 + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 3] = t2_2;

#if !defined(NDEBUG)
  verify_bucket_positions (a1);
  verify_bucket_positions (a2);
#endif

  // final sum up using Sum2s, remember bucket ax[NUMBER_OF_BUCKETS - 1] is
  // always NaN
  double a =
    (a1[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS] +
     a2[BD_NUMBER_OF_BUCKETS - BD_NUMBER_OF_OVERFLOW_BUCKETS]);
  double err = 0.0;

  for (int i = BD_NUMBER_OF_NORMAL_BUCKETS + BD_NUMBER_OF_UNDERFLOW_BUCKETS - 1;
       i >= 0; i--)
    {
      // FastTwoSum
      double b = (a1[i] + a2[i]);
      double x = a + b;
      err += (a - x) + b;
      a = x;
    }
  sum += a + err;

  // prepare for next usage
  initialize_buckets ();

  return sum;
}

