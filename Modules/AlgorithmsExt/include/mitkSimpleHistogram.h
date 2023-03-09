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

#include "MitkAlgorithmsExtExports.h"

#include <mitkImage.h>
#include <mitkWeakPointer.h>

#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

namespace mitk
{
  //##Documentation
  //## @brief Abstract superclass for histograms with double values.
  //##        Classes which are deriving from this class can be cached
  //##        in the same way.
  class MITKALGORITHMSEXT_EXPORT SimpleHistogram
  {
  public:
    /** @brief Returns the minimal value of the histogram. */
    virtual double GetMin() const = 0;
    /** @brief Returns the maximum value of the histogram. */
    virtual double GetMax() const = 0;
    /** @brief Creates a new histogram out the source. */
    virtual void ComputeFromBaseData(BaseData *source) = 0;
    /** @brief TODO: (What should this method do?)*/
    virtual float GetRelativeBin(double start, double end) const = 0;
  };

  class MITKALGORITHMSEXT_EXPORT SimpleImageHistogram : public SimpleHistogram
  {
  public:
    typedef itk::Statistics::Histogram<double> HistogramType;

    SimpleImageHistogram()
    {
      valid = false;
      histogram = nullptr;
    }

    ~SimpleImageHistogram()
    {
      if (histogram)
        delete histogram;
    }

    /** @return Returns if the current histogram is valid, false if not. */
    bool GetValid();

    typedef itk::Image<short, 3> CTImage;
    typedef itk::ImageRegionIterator<CTImage> CTIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<CTImage> CTIteratorIndexType;

    typedef itk::Image<unsigned char, 3> BinImage;
    typedef itk::ImageRegionIterator<BinImage> BinIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<BinImage> BinIteratorIndexType;

    typedef unsigned long CountType;

  protected:
    CountType *histogram;

    bool valid;

    int first;
    int last;
    int min;
    int max;
    CountType highest;
    double invLogHighest;

  public:
    double GetMin() const override
    {
      if (!valid)
        return 0;

      return min;
    }

    double GetMax() const override
    {
      if (!valid)
        return 1;

      return max;
    }

    /** @brief Creates a new histogram out the source which must be an image. Method does nothing if the image is
     * invalid, nullptr, etc.. */
    void ComputeFromBaseData(BaseData *source) override;
    float GetRelativeBin(double start, double end) const override;
  };

  class MITKALGORITHMSEXT_EXPORT SimpleHistogramCache
  {
  public:
    static const unsigned int maxCacheSize = 64;

    class MITKALGORITHMSEXT_EXPORT Element
    {
    public:
      mitk::WeakPointer<BaseData> baseData;
      itk::TimeStamp m_LastUpdateTime;

      virtual ~Element();

      virtual void ComputeFromBaseData(BaseData *baseData) = 0;
      virtual SimpleHistogram *GetHistogram() = 0;
    };

    typedef std::list<Element *> CacheContainer;

    CacheContainer cache;

    SimpleHistogramCache() {}
    ~SimpleHistogramCache() { TrimCache(true); }
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
