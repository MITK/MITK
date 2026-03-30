/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSimpleHistogram_h
#define mitkSimpleHistogram_h

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

#include <MitkAlgorithmsExtExports.h>

#include <mitkImage.h>
#include <mitkWeakPointer.h>

#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

namespace mitk
{
  /**
   * \brief Abstract base class for simple histograms with double values.
   *
   * Provides the interface for computing histograms from BaseData objects and
   * querying minimum/maximum values and relative bin frequencies. Derived classes
   * can be cached using SimpleHistogramCache.
   *
   * \sa SimpleImageHistogram
   * \sa SimpleUnstructuredGridHistogram
   * \sa SimpleHistogramCache
   */
  class MITKALGORITHMSEXT_EXPORT SimpleHistogram
  {
  public:
    /**
     * \brief Returns the minimum value of the histogram.
     * \return The minimum histogram value.
     */
    virtual double GetMin() const = 0;

    /**
     * \brief Returns the maximum value of the histogram.
     * \return The maximum histogram value.
     */
    virtual double GetMax() const = 0;

    /**
     * \brief Compute the histogram from the given BaseData source.
     * \param[in] source The BaseData object to compute the histogram from.
     */
    virtual void ComputeFromBaseData(BaseData *source) = 0;

    /**
     * \brief Get the relative frequency of bins in the range [start, end].
     * \param[in] start The start of the bin range.
     * \param[in] end The end of the bin range.
     * \return The relative frequency as a float in [0, 1].
     */
    virtual float GetRelativeBin(double start, double end) const = 0;
  };

  /**
   * \brief Histogram implementation for mitk::Image data.
   *
   * Computes a histogram from an image using ITK's histogram facilities.
   * The histogram is computed with 256 bins by default.
   *
   * \sa SimpleHistogram
   * \sa SimpleHistogramCache
   */
  class MITKALGORITHMSEXT_EXPORT SimpleImageHistogram : public SimpleHistogram
  {
  public:
    /** \brief The underlying ITK histogram type. */
    typedef itk::Statistics::Histogram<double> HistogramType;

    SimpleImageHistogram()
    {
      valid = false;
    }

    ~SimpleImageHistogram()
    {
    }

    /**
     * \brief Check whether the histogram has been computed successfully.
     * \return True if the histogram is valid, false otherwise.
     */
    bool GetValid();

    typedef itk::Image<short, 3> CTImage;
    typedef itk::ImageRegionIterator<CTImage> CTIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<CTImage> CTIteratorIndexType;

    typedef itk::Image<unsigned char, 3> BinImage;
    typedef itk::ImageRegionIterator<BinImage> BinIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<BinImage> BinIteratorIndexType;

    typedef unsigned long CountType;

  protected:
    HistogramType::ConstPointer histogram; ///< The computed ITK histogram.

    bool valid;                  ///< Whether the histogram is valid.
    unsigned int nBins = 256;    ///< Number of histogram bins.
    double invLogHighest;        ///< Inverse log of the highest bin frequency (for normalization).

  public:
    /** \brief Returns the minimum value of the histogram. */
    double GetMin() const override;

    /** \brief Returns the maximum value of the histogram. */
    double GetMax() const override;

    /**
     * \brief Compute the histogram from an image.
     *
     * Does nothing if the source is invalid, nullptr, or not an Image.
     *
     * \param[in] source The BaseData (must be a mitk::Image) to compute the histogram from.
     */
    void ComputeFromBaseData(BaseData *source) override;

    /** \copydoc SimpleHistogram::GetRelativeBin */
    float GetRelativeBin(double start, double end) const override;
  };

  /**
   * \brief Cache for SimpleHistogram objects to avoid redundant recomputation.
   *
   * Maintains a list of histogram elements associated with BaseData objects.
   * When a histogram for a particular BaseData is requested, the cache checks
   * whether an up-to-date histogram already exists. The cache has a maximum
   * size of 64 entries and trims oldest entries when full.
   *
   * \sa SimpleHistogram
   */
  class MITKALGORITHMSEXT_EXPORT SimpleHistogramCache
  {
  public:
    /** \brief Maximum number of cached histogram entries. */
    static const unsigned int maxCacheSize = 64;

    /**
     * \brief Abstract base element stored in the histogram cache.
     *
     * Each element holds a weak reference to a BaseData and a timestamp
     * for cache invalidation.
     */
    class MITKALGORITHMSEXT_EXPORT Element
    {
    public:
      mitk::WeakPointer<BaseData> baseData; ///< Weak reference to the source data.
      itk::TimeStamp m_LastUpdateTime;      ///< Timestamp of last computation.

      virtual ~Element();

      /**
       * \brief Compute the histogram from the given BaseData.
       * \param[in] baseData The source data.
       */
      virtual void ComputeFromBaseData(BaseData *baseData) = 0;

      /**
       * \brief Get the computed histogram.
       * \return Pointer to the SimpleHistogram.
       */
      virtual SimpleHistogram *GetHistogram() = 0;
    };

    typedef std::list<Element *> CacheContainer;

    CacheContainer cache; ///< The list of cached elements.

    SimpleHistogramCache() {}
    ~SimpleHistogramCache() { TrimCache(true); }

    /**
     * \brief Retrieve or compute a histogram for the given BaseData.
     * \param[in] sp_BaseData Smart pointer to the BaseData.
     * \return Pointer to the cached or newly computed SimpleHistogram.
     */
    SimpleHistogram *operator[](BaseData::Pointer sp_BaseData);

  protected:
    void TrimCache(bool full = false)
    {
      unsigned int targetSize = full ? 0 : maxCacheSize;

      while (cache.size() > targetSize)
      {
        delete cache.back();
        cache.pop_back();
      }
    }
  };
}

#endif
