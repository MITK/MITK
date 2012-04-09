/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKSIMPLEHISTOGRAM_H
#define MITKSIMPLEHISTOGRAM_H


#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

#include "MitkExtExports.h"

#include <mitkImage.h>
#include <mitkWeakPointer.h>

#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

namespace mitk {


//##Documentation
//## @brief Abstract superclass for histograms with double values.
//##        Classes which are deriving from this class can be cached
//##        in the same way.
class MitkExt_EXPORT SimpleHistogram
{
public:

  /** @brief Returns the minimal value of the histogram. */
  virtual double GetMin() const = 0;
  /** @brief Returns the maximum value of the histogram. */
  virtual double GetMax() const = 0;
  /** @brief Creates a new histogram out the source. */
  virtual void ComputeFromBaseData( BaseData* source ) = 0;
  /** @brief TODO: (What should this method do?)*/
  virtual float GetRelativeBin( double start, double end ) const = 0;
};

class MitkExt_EXPORT SimpleImageHistogram : public SimpleHistogram
{
  public:

  typedef itk::Statistics::Histogram<double> HistogramType;

  SimpleImageHistogram()
  {
    valid=false;
    histogram=0;
  }

  ~SimpleImageHistogram()
  {
    if(histogram)
      delete histogram;
  }

  /** @return Returns if the current histogram is valid, false if not. */
  bool GetValid();

  typedef itk::Image<short, 3>          CTImage;
  typedef itk::ImageRegionIterator< CTImage  > CTIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< CTImage  > CTIteratorIndexType;

  typedef itk::Image<unsigned char, 3> BinImage;
  typedef itk::ImageRegionIterator< BinImage > BinIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< BinImage > BinIteratorIndexType;

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

  double GetMin() const
  {
    if(!valid)
      return 0;

    return min;
  }

  double GetMax() const
  {
    if(!valid)
      return 1;

    return max;
  }

  /** @brief Creates a new histogram out the source which must be an image. Method does nothing if the image is invalid, NULL, etc.. */
  void ComputeFromBaseData( BaseData* source );
  float GetRelativeBin( double start, double end ) const;

};

class MitkExt_EXPORT SimpleHistogramCache
{
  public:

    static const unsigned int maxCacheSize = 64;

    class Element
    {
      public:
        mitk::WeakPointer<BaseData> baseData;
        itk::TimeStamp m_LastUpdateTime;

        virtual void ComputeFromBaseData(BaseData* baseData) = 0;
        virtual SimpleHistogram* GetHistogram() = 0;
    };

    typedef std::list<Element*> CacheContainer;

    CacheContainer cache;

    SimpleHistogramCache()
    {

    }

    ~SimpleHistogramCache()
    {
      TrimCache(true);
    }

    SimpleHistogram *operator[](BaseData::Pointer sp_BaseData);

  protected:

    void TrimCache(bool full=false)
    {
      unsigned int targetSize = full?0:maxCacheSize;

      while(cache.size()>targetSize)
      {
        delete cache.back();
        cache.pop_back();
      }

    }
};

}

#endif // MITKSIMPLEHISTOGRAM_H
