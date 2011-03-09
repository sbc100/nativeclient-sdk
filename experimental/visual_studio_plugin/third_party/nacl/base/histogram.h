// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Histogram is an object that aggregates statistics, and can summarize them in
// various forms, including ASCII graphical, HTML, and numerically (as a
// vector of numbers corresponding to each of the aggregating buckets).

// It supports calls to accumulate either time intervals (which are processed
// as integral number of milliseconds), or arbitrary integral units.

// The default layout of buckets is exponential.  For example, buckets might
// contain (sequentially) the count of values in the following intervals:
// [0,1), [1,2), [2,4), [4,8), [8,16), [16,32), [32,64), [64,infinity)
// That bucket allocation would actually result from construction of a histogram
// for values between 1 and 64, with 8 buckets, such as:
// Histogram count(L"some name", 1, 64, 8);
// Note that the underflow bucket [0,1) and the overflow bucket [64,infinity)
// are not counted by the constructor in the user supplied "bucket_count"
// argument.
// The above example has an exponential ratio of 2 (doubling the bucket width
// in each consecutive bucket.  The Histogram class automatically calculates
// the smallest ratio that it can use to construct the number of buckets
// selected in the constructor.  An another example, if you had 50 buckets,
// and millisecond time values from 1 to 10000, then the ratio between
// consecutive bucket widths will be approximately somewhere around the 50th
// root of 10000.  This approach provides very fine grain (narrow) buckets
// at the low end of the histogram scale, but allows the histogram to cover a
// gigantic range with the addition of very few buckets.

#ifndef BASE_HISTOGRAM_H_
#define BASE_HISTOGRAM_H_

#include <map>
#include <string>
#include <vector>

#include "base/lock.h"
#include "base/ref_counted.h"
#include "base/logging.h"
#include "base/time.h"

//------------------------------------------------------------------------------
// Provide easy general purpose histogram in a macro, just like stats counters.
// The first four macros use 50 buckets.

#define HISTOGRAM_TIMES(name, sample) HISTOGRAM_CUSTOM_TIMES( \
    name, sample, base::TimeDelta::FromMilliseconds(1), \
    base::TimeDelta::FromSeconds(10), 50)

#define HISTOGRAM_COUNTS(name, sample) HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1, 1000000, 50)

#define HISTOGRAM_COUNTS_100(name, sample) HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1, 100, 50)

#define HISTOGRAM_COUNTS_10000(name, sample) HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1, 10000, 50)

#define HISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count) do { \
    static scoped_refptr<Histogram> counter = Histogram::FactoryGet( \
        name, min, max, bucket_count, Histogram::kNoFlags); \
    counter->Add(sample); \
  } while (0)

#define HISTOGRAM_PERCENTAGE(name, under_one_hundred) \
    HISTOGRAM_ENUMERATION(name, under_one_hundred, 101)

// For folks that need real specific times, use this to select a precise range
// of times you want plotted, and the number of buckets you want used.
#define HISTOGRAM_CUSTOM_TIMES(name, sample, min, max, bucket_count) do { \
    static scoped_refptr<Histogram> counter = Histogram::FactoryGet( \
        name, min, max, bucket_count, Histogram::kNoFlags); \
    counter->AddTime(sample); \
  } while (0)

// DO NOT USE THIS.  It is being phased out, in favor of HISTOGRAM_CUSTOM_TIMES.
#define HISTOGRAM_CLIPPED_TIMES(name, sample, min, max, bucket_count) do { \
    static scoped_refptr<Histogram> counter = Histogram::FactoryGet( \
        name, min, max, bucket_count, Histogram::kNoFlags); \
    if ((sample) < (max)) counter->AddTime(sample); \
  } while (0)

// Support histograming of an enumerated value.  The samples should always be
// less than boundary_value.

#define HISTOGRAM_ENUMERATION(name, sample, boundary_value) do { \
    static scoped_refptr<Histogram> counter = LinearHistogram::FactoryGet( \
        name, 1, boundary_value, boundary_value + 1, Histogram::kNoFlags); \
    counter->Add(sample); \
  } while (0)


//------------------------------------------------------------------------------
// Define Debug vs non-debug flavors of macros.
#ifndef NDEBUG

#define DHISTOGRAM_TIMES(name, sample) HISTOGRAM_TIMES(name, sample)
#define DHISTOGRAM_COUNTS(name, sample) HISTOGRAM_COUNTS(name, sample)
#define DHISTOGRAM_PERCENTAGE(name, under_one_hundred) HISTOGRAM_PERCENTAGE(\
    name, under_one_hundred)
#define DHISTOGRAM_CUSTOM_TIMES(name, sample, min, max, bucket_count) \
    HISTOGRAM_CUSTOM_TIMES(name, sample, min, max, bucket_count)
#define DHISTOGRAM_CLIPPED_TIMES(name, sample, min, max, bucket_count) \
    HISTOGRAM_CLIPPED_TIMES(name, sample, min, max, bucket_count)
#define DHISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count) \
    HISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count)
#define DHISTOGRAM_ENUMERATION(name, sample, boundary_value) \
    HISTOGRAM_ENUMERATION(name, sample, boundary_value)

#else  // NDEBUG

#define DHISTOGRAM_TIMES(name, sample) do {} while (0)
#define DHISTOGRAM_COUNTS(name, sample) do {} while (0)
#define DHISTOGRAM_PERCENTAGE(name, under_one_hundred) do {} while (0)
#define DHISTOGRAM_CUSTOM_TIMES(name, sample, min, max, bucket_count) \
    do {} while (0)
#define DHISTOGRAM_CLIPPED_TIMES(name, sample, min, max, bucket_count) \
    do {} while (0)
#define DHISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count) \
    do {} while (0)
#define DHISTOGRAM_ENUMERATION(name, sample, boundary_value) do {} while (0)

#endif  // NDEBUG

//------------------------------------------------------------------------------
// The following macros provide typical usage scenarios for callers that wish
// to record histogram data, and have the data submitted/uploaded via UMA.
// Not all systems support such UMA, but if they do, the following macros
// should work with the service.

#define UMA_HISTOGRAM_TIMES(name, sample) UMA_HISTOGRAM_CUSTOM_TIMES( \
    name, sample, base::TimeDelta::FromMilliseconds(1), \
    base::TimeDelta::FromSeconds(10), 50)

#define UMA_HISTOGRAM_MEDIUM_TIMES(name, sample) UMA_HISTOGRAM_CUSTOM_TIMES( \
    name, sample, base::TimeDelta::FromMilliseconds(10), \
    base::TimeDelta::FromMinutes(3), 50)

// Use this macro when times can routinely be much longer than 10 seconds.
#define UMA_HISTOGRAM_LONG_TIMES(name, sample) UMA_HISTOGRAM_CUSTOM_TIMES( \
    name, sample, base::TimeDelta::FromMilliseconds(1), \
    base::TimeDelta::FromHours(1), 50)

#define UMA_HISTOGRAM_CUSTOM_TIMES(name, sample, min, max, bucket_count) do { \
    static scoped_refptr<Histogram> counter = Histogram::FactoryGet( \
        name, min, max, bucket_count, Histogram::kUmaTargetedHistogramFlag); \
    counter->AddTime(sample); \
  } while (0)

// DO NOT USE THIS.  It is being phased out, in favor of HISTOGRAM_CUSTOM_TIMES.
#define UMA_HISTOGRAM_CLIPPED_TIMES(name, sample, min, max, bucket_count) do { \
    static scoped_refptr<Histogram> counter = Histogram::FactoryGet( \
        name, min, max, bucket_count, Histogram::kUmaTargetedHistogramFlag); \
    if ((sample) < (max)) counter->AddTime(sample); \
  } while (0)

#define UMA_HISTOGRAM_COUNTS(name, sample) UMA_HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1, 1000000, 50)

#define UMA_HISTOGRAM_COUNTS_100(name, sample) UMA_HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1, 100, 50)

#define UMA_HISTOGRAM_COUNTS_10000(name, sample) UMA_HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1, 10000, 50)

#define UMA_HISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count) do { \
    static scoped_refptr<Histogram> counter = Histogram::FactoryGet( \
        name, min, max, bucket_count, Histogram::kUmaTargetedHistogramFlag); \
    counter->Add(sample); \
  } while (0)

#define UMA_HISTOGRAM_MEMORY_KB(name, sample) UMA_HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1000, 500000, 50)

#define UMA_HISTOGRAM_MEMORY_MB(name, sample) UMA_HISTOGRAM_CUSTOM_COUNTS( \
    name, sample, 1, 1000, 50)

#define UMA_HISTOGRAM_PERCENTAGE(name, under_one_hundred) \
    UMA_HISTOGRAM_ENUMERATION(name, under_one_hundred, 101)

#define UMA_HISTOGRAM_ENUMERATION(name, sample, boundary_value) do { \
    static scoped_refptr<Histogram> counter = LinearHistogram::FactoryGet( \
        name, 1, boundary_value, boundary_value + 1, \
        Histogram::kUmaTargetedHistogramFlag); \
    counter->Add(sample); \
  } while (0)


//------------------------------------------------------------------------------

class Pickle;
class Histogram;
class LinearHistogram;
class BooleanHistogram;

namespace disk_cache {
  class StatsHistogram;
};  // namespace disk_cache


class Histogram : public base::RefCountedThreadSafe<Histogram> {
 public:
  typedef int Sample;  // Used for samples (and ranges of samples).
  typedef int Count;  // Used to count samples in a bucket.
  static const Sample kSampleType_MAX = INT_MAX;

  typedef std::vector<Count> Counts;
  typedef std::vector<Sample> Ranges;

  /* These enums are meant to facilitate deserialization of renderer histograms
     into the browser. */
  enum ClassType {
    HISTOGRAM,
    LINEAR_HISTOGRAM,
    BOOLEAN_HISTOGRAM,
    NOT_VALID_IN_RENDERER
  };

  enum BucketLayout {
    EXPONENTIAL,
    LINEAR
  };

  enum Flags {
    kNoFlags = 0,
    kUmaTargetedHistogramFlag = 0x1,  // Histogram should be UMA uploaded.

    // Indicate that the histogram was pickled to be sent across an IPC Channel.
    // If we observe this flag on a histogram being aggregated into after IPC,
    // then we are running in a single process mode, and the aggregation should
    // not take place (as we would be aggregating back into the source
    // histogram!).
    kIPCSerializationSourceFlag = 0x10,

    kHexRangePrintingFlag = 0x8000,  // Fancy bucket-naming supported.
  };

  struct DescriptionPair {
    Sample sample;
    const char* description;  // Null means end of a list of pairs.
  };

  //----------------------------------------------------------------------------
  // Statistic values, developed over the life of the histogram.

  class SampleSet {
   public:
    explicit SampleSet();
    // Adjust size of counts_ for use with given histogram.
    void Resize(const Histogram& histogram);
    void CheckSize(const Histogram& histogram) const;

    // Accessor for histogram to make routine additions.
    void Accumulate(Sample value, Count count, size_t index);

    // Accessor methods.
    Count counts(size_t i) const { return counts_[i]; }
    Count TotalCount() const;
    int64 sum() const { return sum_; }
    int64 square_sum() const { return square_sum_; }

    // Arithmetic manipulation of corresponding elements of the set.
    void Add(const SampleSet& other);
    void Subtract(const SampleSet& other);

    bool Serialize(Pickle* pickle) const;
    bool Deserialize(void** iter, const Pickle& pickle);

   protected:
    // Actual histogram data is stored in buckets, showing the count of values
    // that fit into each bucket.
    Counts counts_;

    // Save simple stats locally.  Note that this MIGHT get done in base class
    // without shared memory at some point.
    int64 sum_;         // sum of samples.
    int64 square_sum_;  // sum of squares of samples.
  };
  //----------------------------------------------------------------------------
  // minimum should start from 1. 0 is invalid as a minimum. 0 is an implicit
  // default underflow bucket.
  static scoped_refptr<Histogram> FactoryGet(const std::string& name,
      Sample minimum, Sample maximum, size_t bucket_count, Flags flags);
  static scoped_refptr<Histogram> FactoryGet(const std::string& name,
      base::TimeDelta minimum, base::TimeDelta maximum, size_t bucket_count,
      Flags flags);

  void Add(int value);

  // This method is an interface, used only by BooleanHistogram.
  virtual void AddBoolean(bool value) { DCHECK(false); }

  // Accept a TimeDelta to increment.
  void AddTime(base::TimeDelta time) {
    Add(static_cast<int>(time.InMilliseconds()));
  }

  void AddSampleSet(const SampleSet& sample);

  // This method is an interface, used only by LinearHistogram.
  virtual void SetRangeDescriptions(const DescriptionPair descriptions[])
      { DCHECK(false); }

  // The following methods provide graphical histogram displays.
  void WriteHTMLGraph(std::string* output) const;
  void WriteAscii(bool graph_it, const std::string& newline,
                  std::string* output) const;

  // Support generic flagging of Histograms.
  // 0x1 Currently used to mark this histogram to be recorded by UMA..
  // 0x8000 means print ranges in hex.
  void SetFlags(Flags flags) { flags_ = static_cast<Flags> (flags_ | flags); }
  void ClearFlags(Flags flags) { flags_ = static_cast<Flags>(flags_ & ~flags); }
  int flags() const { return flags_; }

  // Convenience methods for serializing/deserializing the histograms.
  // Histograms from Renderer process are serialized and sent to the browser.
  // Browser process reconstructs the histogram from the pickled version
  // accumulates the browser-side shadow copy of histograms (that mirror
  // histograms created in the renderer).

  // Serialize the given snapshot of a Histogram into a String. Uses
  // Pickle class to flatten the object.
  static std::string SerializeHistogramInfo(const Histogram& histogram,
                                            const SampleSet& snapshot);
  // The following method accepts a list of pickled histograms and
  // builds a histogram and updates shadow copy of histogram data in the
  // browser process.
  static bool DeserializeHistogramInfo(const std::string& histogram_info);

  //----------------------------------------------------------------------------
  // Accessors for factory constuction, serialization and testing.
  //----------------------------------------------------------------------------
  virtual ClassType histogram_type() const { return HISTOGRAM; }
  const std::string histogram_name() const { return histogram_name_; }
  Sample declared_min() const { return declared_min_; }
  Sample declared_max() const { return declared_max_; }
  virtual Sample ranges(size_t i) const { return ranges_[i];}
  virtual size_t bucket_count() const { return bucket_count_; }
  // Snapshot the current complete set of sample data.
  // Override with atomic/locked snapshot if needed.
  virtual void SnapshotSample(SampleSet* sample) const;

  virtual bool HasConstructorArguments(Sample minimum, Sample maximum,
      size_t bucket_count) {
    return ((minimum == declared_min_) && (maximum == declared_max_) &&
            (bucket_count == bucket_count_));
  }

  virtual bool HasConstructorTimeDeltaArguments(base::TimeDelta minimum,
      base::TimeDelta maximum, size_t bucket_count) {
    return ((minimum.InMilliseconds() == declared_min_) &&
            (maximum.InMilliseconds() == declared_max_) &&
            (bucket_count == bucket_count_));
  }

 protected:
  friend class base::RefCountedThreadSafe<Histogram>;
  Histogram(const std::string& name, Sample minimum,
            Sample maximum, size_t bucket_count);
  Histogram(const std::string& name, base::TimeDelta minimum,
            base::TimeDelta maximum, size_t bucket_count);

  virtual ~Histogram();

  // Method to override to skip the display of the i'th bucket if it's empty.
  virtual bool PrintEmptyBucket(size_t index) const { return true; }

  //----------------------------------------------------------------------------
  // Methods to override to create histogram with different bucket widths.
  //----------------------------------------------------------------------------
  // Initialize ranges_ mapping.
  virtual void InitializeBucketRange();
  // Find bucket to increment for sample value.
  virtual size_t BucketIndex(Sample value) const;
  // Get normalized size, relative to the ranges_[i].
  virtual double GetBucketSize(Count current, size_t i) const;

  // Return a string description of what goes in a given bucket.
  // Most commonly this is the numeric value, but in derived classes it may
  // be a name (or string description) given to the bucket.
  virtual const std::string GetAsciiBucketRange(size_t it) const;

  //----------------------------------------------------------------------------
  // Methods to override to create thread safe histogram.
  //----------------------------------------------------------------------------
  // Update all our internal data, including histogram
  virtual void Accumulate(Sample value, Count count, size_t index);

  //----------------------------------------------------------------------------
  // Accessors for derived classes.
  //----------------------------------------------------------------------------
  void SetBucketRange(size_t i, Sample value);

  // Validate that ranges_ was created sensibly (top and bottom range
  // values relate properly to the declared_min_ and declared_max_)..
  bool ValidateBucketRanges() const;

 private:
  // Post constructor initialization.
  void Initialize();

  //----------------------------------------------------------------------------
  // Helpers for emitting Ascii graphic.  Each method appends data to output.

  // Find out how large the (graphically) the largest bucket will appear to be.
  double GetPeakBucketSize(const SampleSet& snapshot) const;

  // Write a common header message describing this histogram.
  void WriteAsciiHeader(const SampleSet& snapshot,
                        Count sample_count, std::string* output) const;

  // Write information about previous, current, and next buckets.
  // Information such as cumulative percentage, etc.
  void WriteAsciiBucketContext(const int64 past, const Count current,
                               const int64 remaining, const size_t i,
                               std::string* output) const;

  // Write textual description of the bucket contents (relative to histogram).
  // Output is the count in the buckets, as well as the percentage.
  void WriteAsciiBucketValue(Count current, double scaled_sum,
                             std::string* output) const;

  // Produce actual graph (set of blank vs non blank char's) for a bucket.
  void WriteAsciiBucketGraph(double current_size, double max_size,
                             std::string* output) const;

  //----------------------------------------------------------------------------
  // Invariant values set at/near construction time

  // ASCII version of original name given to the constructor.  All identically
  // named instances will  be coalesced cross-project TODO(jar).
  // If a user needs one histogram name to be called by several places in a
  // single process, a central function should be defined by the user, which
  // defins the single declared instance of the named histogram.
  const std::string histogram_name_;
  Sample declared_min_;  // Less than this goes into counts_[0]
  Sample declared_max_;  // Over this goes into counts_[bucket_count_ - 1].
  size_t bucket_count_;  // Dimension of counts_[].

  // Flag the histogram for recording by UMA via metric_services.h.
  Flags flags_;

  // For each index, show the least value that can be stored in the
  // corresponding bucket. We also append one extra element in this array,
  // containing kSampleType_MAX, to make calculations easy.
  // The dimension of ranges_ is bucket_count + 1.
  Ranges ranges_;

  // Finally, provide the state that changes with the addition of each new
  // sample.
  SampleSet sample_;

  DISALLOW_COPY_AND_ASSIGN(Histogram);
};

//------------------------------------------------------------------------------

// LinearHistogram is a more traditional histogram, with evenly spaced
// buckets.
class LinearHistogram : public Histogram {
 public:
  virtual ClassType histogram_type() const { return LINEAR_HISTOGRAM; }

  // Store a list of number/text values for use in rendering the histogram.
  // The last element in the array has a null in its "description" slot.
  virtual void SetRangeDescriptions(const DescriptionPair descriptions[]);

  /* minimum should start from 1. 0 is as minimum is invalid. 0 is an implicit
     default underflow bucket. */
  static scoped_refptr<Histogram> FactoryGet(const std::string& name,
      Sample minimum, Sample maximum, size_t bucket_count, Flags flags);
  static scoped_refptr<Histogram> FactoryGet(const std::string& name,
      base::TimeDelta minimum, base::TimeDelta maximum, size_t bucket_count,
      Flags flags);

 protected:
  LinearHistogram(const std::string& name, Sample minimum,
                  Sample maximum, size_t bucket_count);

  LinearHistogram(const std::string& name, base::TimeDelta minimum,
                  base::TimeDelta maximum, size_t bucket_count);

  virtual ~LinearHistogram() {}

  // Initialize ranges_ mapping.
  virtual void InitializeBucketRange();
  virtual double GetBucketSize(Count current, size_t i) const;

  // If we have a description for a bucket, then return that.  Otherwise
  // let parent class provide a (numeric) description.
  virtual const std::string GetAsciiBucketRange(size_t i) const;

  // Skip printing of name for numeric range if we have a name (and if this is
  // an empty bucket).
  virtual bool PrintEmptyBucket(size_t index) const;

 private:
  // For some ranges, we store a printable description of a bucket range.
  // If there is no desciption, then GetAsciiBucketRange() uses parent class
  // to provide a description.
  typedef std::map<Sample, std::string> BucketDescriptionMap;
  BucketDescriptionMap bucket_description_;

  DISALLOW_COPY_AND_ASSIGN(LinearHistogram);
};

//------------------------------------------------------------------------------

// BooleanHistogram is a histogram for booleans.
class BooleanHistogram : public LinearHistogram {
 public:
  static scoped_refptr<Histogram> FactoryGet(const std::string& name,
      Flags flags);

  virtual ClassType histogram_type() const { return BOOLEAN_HISTOGRAM; }

  virtual void AddBoolean(bool value) { Add(value ? 1 : 0); }

 private:
  explicit BooleanHistogram(const std::string& name)
      : LinearHistogram(name, 1, 2, 3) {
  }

  DISALLOW_COPY_AND_ASSIGN(BooleanHistogram);
};

//------------------------------------------------------------------------------
// StatisticsRecorder handles all histograms in the system.  It provides a
// general place for histograms to register, and supports a global API for
// accessing (i.e., dumping, or graphing) the data in all the histograms.

class StatisticsRecorder {
 public:
  typedef std::vector<scoped_refptr<Histogram> > Histograms;

  StatisticsRecorder();

  ~StatisticsRecorder();

  // Find out if histograms can now be registered into our list.
  static bool WasStarted();

  // Register, or add a new histogram to the collection of statistics.
  static void Register(Histogram* histogram);

  // Methods for printing histograms.  Only histograms which have query as
  // a substring are written to output (an empty string will process all
  // registered histograms).
  static void WriteHTMLGraph(const std::string& query, std::string* output);
  static void WriteGraph(const std::string& query, std::string* output);

  // Method for extracting histograms which were marked for use by UMA.
  static void GetHistograms(Histograms* output);

  // Find a histogram by name. It matches the exact name. This method is thread
  // safe.
  static bool FindHistogram(const std::string& query,
                            scoped_refptr<Histogram>* histogram);

  static bool dump_on_exit() { return dump_on_exit_; }

  static void set_dump_on_exit(bool enable) { dump_on_exit_ = enable; }

  // GetSnapshot copies some of the pointers to registered histograms into the
  // caller supplied vector (Histograms).  Only histograms with names matching
  // query are returned. The query must be a substring of histogram name for its
  // pointer to be copied.
  static void GetSnapshot(const std::string& query, Histograms* snapshot);


 private:
  // We keep all registered histograms in a map, from name to histogram.
  typedef std::map<std::string, scoped_refptr<Histogram> > HistogramMap;

  static HistogramMap* histograms_;

  // lock protects access to the above map.
  static Lock* lock_;

  // Dump all known histograms to log.
  static bool dump_on_exit_;

  DISALLOW_COPY_AND_ASSIGN(StatisticsRecorder);
};

#endif  // BASE_HISTOGRAM_H_
