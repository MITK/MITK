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
#ifndef __itkTdiToVolumeFractionFilter_h_
#define __itkTdiToVolumeFractionFilter_h_

#include <MitkFiberTrackingExports.h>
#include <itkImageToImageFilter.h>

namespace itk{

/**
* \brief  */

template< class TPixelType >
class TdiToVolumeFractionFilter : public ImageToImageFilter< Image< TPixelType, 3 >, Image< TPixelType, 3 > >
{

public:

  typedef TdiToVolumeFractionFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< Image< TPixelType, 3 >, Image< TPixelType, 3 > > Superclass;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(TdiToVolumeFractionFilter, ImageToImageFilter)

  typedef typename Superclass::InputImageType         InputImageType;
  typedef typename Superclass::OutputImageType        OutputImageType;
  typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;

  itkSetMacro( TdiThreshold, TPixelType )
  itkSetMacro( Sqrt, TPixelType )

protected:
  TdiToVolumeFractionFilter();
  ~TdiToVolumeFractionFilter() override {}

  void BeforeThreadedGenerateData() override;
  void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) override;

private:

  TPixelType m_TdiThreshold;
  TPixelType m_Sqrt;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTdiToVolumeFractionFilter.cpp"
#endif

#endif //__itkTdiToVolumeFractionFilter_h_

