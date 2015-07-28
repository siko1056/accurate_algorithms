#include "BucketSum.hpp"

#if !defined(NDEBUG)
/**
 * Helper function to detect missalignment of the buckets.
 */
inline void
verify_bucket_positions (double* buckets)
{
  assert (((BucketSum::ieee754_double *) & buckets[0])->exponent <=
          (std::numeric_limits<double>::min_exponent +
           (std::numeric_limits<double>::max_exponent - 1)));
  for (int i = 1; i < (NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS); i++)
    {
      assert (((BucketSum::ieee754_double *) & buckets[i])->exponent ==
              ((std::numeric_limits<double>::min_exponent - 1) +
               (i * SHIFT) - 1 +
               (std::numeric_limits<double>::max_exponent - 1)));
    }

  assert (((BucketSum::ieee754_double *) &
           buckets[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS])->
          exponent == 2 * (std::numeric_limits<double>::max_exponent - 1));
  // buckets[NUMBER_OF_BUCKETS - 1] is always NaN
}
#endif

/**
 * Constructor
 */
BucketSum::BucketSum ()
{
  // allocate bucket memory
  a1 = new double[NUMBER_OF_BUCKETS];
  a2 = new double[NUMBER_OF_BUCKETS];

  // create bucket masks
  mask = new double[NUMBER_OF_BUCKETS];
  mask[0] = 0.0;
  mask[1] =
    1.5 * std::pow (2.0, (std::numeric_limits<double>::min_exponent - 1) +
                    (SHIFT - 1));
  for (int i = 2; i < (NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS); i++)
    mask[i] = mask[i - 1] * std::pow (2.0, SHIFT);
  mask[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS] =
    1.5 * std::pow (2.0, (std::numeric_limits<double>::max_exponent - 1));
  mask[NUMBER_OF_BUCKETS - 1] = std::numeric_limits<double>::quiet_NaN();

  // prepare buckets
  initialize_buckets ();
}

/**
 * Destructor
 */
BucketSum::~BucketSum ()
{
  delete [] a1;
  delete [] a2;
  delete [] mask;
}

void
BucketSum::initialize_buckets ()
{
  for (int i = 0; i < NUMBER_OF_BUCKETS; i++)
    a1[i] = mask[i];
  for (int i = 0; i < NUMBER_OF_BUCKETS; i++)
    a2[i] = -mask[i];

#if !defined(NDEBUG)
  verify_bucket_positions (a1);
  verify_bucket_positions (a2);
#endif
}

/**
 * Sums up all elements of abritary condition number with fixed memory usage.
 *
 * @param input array of addends
 * @param n length of the array of addends
 * @return correctly rounded sum of the addends
 */
double
BucketSum::sum (double *input, int n)
{
  double sum = 0.0;

  // check input, main loop preparation requires more than one addend
  if (n < 1)
    return sum;
  else if (n == 1)
    return input[0];

  // if n is odd, treat first addend seperately
  if (n & 1)
    {
      unsigned pos = ((ieee754_double *) (&input[0]))->exponent;
      pos = ((pos * DIV_18_MULTIPLIER) >> DIV_18_SHIFT);
      double t = a1[pos + NUMBER_OF_UNDERFLOW_BUCKETS] + input[0];
      a1[pos] += (a1[pos + NUMBER_OF_UNDERFLOW_BUCKETS] - t) + input[0];
      a1[pos + NUMBER_OF_UNDERFLOW_BUCKETS] = t;
      input++;
      n--;
    }

  // prepare accumulation of remaining addends
  int ov_counter = 1;  // counter for overflow tidy up
  unsigned pos1 = ((ieee754_double *) (&input[0]))->exponent;
  unsigned pos2 = ((ieee754_double *) (&input[1]))->exponent;
  pos1 = ((pos1 * DIV_18_MULTIPLIER) >> DIV_18_SHIFT);
  pos2 = ((pos2 * DIV_18_MULTIPLIER) >> DIV_18_SHIFT);

  // main loop for accumulation
  while (1)
    {
      // number of addend till end-2 of input array or next tidy-up
      int limit = std::min (ACCUMULATION_RESERVE_OVERFLOW, n - 2);

      // fast loop over even numbers of addends -> add into 2 accumulators
      for (int i = 0; i < limit; i += 2)
        {
          // two parallel FastTwoSum and extraction of the next two exponents
          double t1 = a1[pos1 + NUMBER_OF_UNDERFLOW_BUCKETS] + input[i];
          double t2 = a2[pos2 + NUMBER_OF_UNDERFLOW_BUCKETS] + input[i + 1];

          // interleaved preparation for next loop
          unsigned pos1_new = ((ieee754_double *) (&input[(i + 2)]))->exponent;
          unsigned pos2_new = ((ieee754_double *) (&input[(i + 3)]))->exponent;
          pos1_new = ((pos1_new * DIV_18_MULTIPLIER) >> DIV_18_SHIFT);
          pos2_new = ((pos2_new * DIV_18_MULTIPLIER) >> DIV_18_SHIFT);

          a1[pos1] += (a1[pos1 + NUMBER_OF_UNDERFLOW_BUCKETS] - t1) + input[i];
          a2[pos2] += (a2[pos2 + NUMBER_OF_UNDERFLOW_BUCKETS] - t2)
                      + input[i + 1];
          a1[pos1 + NUMBER_OF_UNDERFLOW_BUCKETS] = t1;
          a2[pos2 + NUMBER_OF_UNDERFLOW_BUCKETS] = t2;

          pos1 = pos1_new;
          pos2 = pos2_new;
        }

      // leave loop if no input entries left
      if (limit == (n - 2))
        break;

      // update input position and number of remaining addends
      input += limit;
      n -= limit;
      ov_counter++;

      // tidy up accumulation array
      if (ov_counter *  ACCUMULATION_RESERVE_OVERFLOW > ACCUMULATION_RESERVE)
        {
          for (int i = 0; i < (NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS);
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
        (a1[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS] +
         a2[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS]);
      a1[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS] =
        mask[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS];
      a2[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS] =
        -mask[NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS];
    }

  // treat last two addends seperately to avoid array bound exceeding
  // two parallel FastTwoSum
  double t1 = a1[pos1 + NUMBER_OF_UNDERFLOW_BUCKETS] + input[n - 2];
  double t2 = a2[pos2 + NUMBER_OF_UNDERFLOW_BUCKETS] + input[n - 1];
  a1[pos1] += (a1[pos1 + NUMBER_OF_UNDERFLOW_BUCKETS] - t1) + input[n - 2];
  a2[pos2] += (a2[pos2 + NUMBER_OF_UNDERFLOW_BUCKETS] - t2) + input[n - 1];
  a1[pos1 + NUMBER_OF_UNDERFLOW_BUCKETS] = t1;
  a2[pos2 + NUMBER_OF_UNDERFLOW_BUCKETS] = t2;

#if !defined(NDEBUG)
  verify_bucket_positions (a1);
  verify_bucket_positions (a2);
#endif

  // final sum up using Sum2s, remember bucket ax[NUMBER_OF_BUCKETS - 1] is
  // always NaN
  double err = 0.0;

  for (int i = NUMBER_OF_BUCKETS - NUMBER_OF_OVERFLOW_BUCKETS;
       i >= 0; i--)
    {
      // FastTwoSum
      double b = (a1[i] + a2[i]);
      double x = sum + b;
      err += (sum - x) + b;
      sum = x;
    }
  sum += err;

  // prepare for next usage
  initialize_buckets ();

  return sum;
}

