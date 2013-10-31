/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkNonLocalMeansDenoisingFilter_h_
#define __itkNonLocalMeansDenoisingFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionImage.h>
#include <itkNeighborhoodIterator.h>
#include <itkVectorImageToImageFilter.h>
#include <itkStatisticsImageFilter.h>
#include <boost/progress.hpp>

namespace itk{
  /** \class NonLocalMeansDenoisingFilter
  */

  template< class TInPixelType, class TOutPixelType >
  class NonLocalMeansDenoisingFilter :
    public ImageToImageFilter< VectorImage < TInPixelType, 3 >, VectorImage < TOutPixelType, 3 > >
  {
  public:

    typedef NonLocalMeansDenoisingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage < TInPixelType, 3 >, VectorImage < TOutPixelType, 3 > >  Superclass;
    typedef VectorImageToImageFilter < TInPixelType > imageExtractorType;
    typedef StatisticsImageFilter < itk::Image<TInPixelType, 3> > StatisticsFilterType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

      /** Runtime information support. */
      itkTypeMacro(NonLocalMeansDenoisingFilter, ImageToImageFilter)

      typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    void SetNRadius(unsigned int n);
    void SetVRadius(unsigned int v);
    //void SetProgressbar(

  protected:
    NonLocalMeansDenoisingFilter();
    ~NonLocalMeansDenoisingFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

    typename NeighborhoodIterator < VectorImage < TInPixelType, 3 > >::RadiusType m_V_Radius;
    typename NeighborhoodIterator < VectorImage < TInPixelType, 3 > >::RadiusType m_N_Radius;
    itk::VariableLengthVector < double > deviations;

    unsigned long m_Pixels;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNonLocalMeansDenoisingFilter.cpp"
#endif

#endif //__itkNonLocalMeansDenoisingFilter_h_
