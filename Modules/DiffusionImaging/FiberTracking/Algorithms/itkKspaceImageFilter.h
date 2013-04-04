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
#ifndef __itkKspaceImageFilter_h_
#define __itkKspaceImageFilter_h_

#include "FiberTrackingExports.h"
#include <itkImageToImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <vcl_complex.h>

namespace itk{

/**
* \brief Performes deterministic streamline tracking on the input tensor image.   */

  template< class TPixelType >
  class KspaceImageFilter :
      public ImageToImageFilter< Image< TPixelType >, Image< vcl_complex< TPixelType >  > >
  {

  public:

    typedef KspaceImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< TPixelType >, Image< vcl_complex< TPixelType >  > > Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(KspaceImageFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType         InputImageType;
    typedef typename Superclass::OutputImageType        OutputImageType;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;

    itkGetMacro( SpectrumImage, typename InputImageType::Pointer )

    itkSetMacro( FrequencyImage, typename InputImageType::Pointer )
    itkSetMacro( tLine, double )
    itkSetMacro( kOffset, double )

  protected:
    KspaceImageFilter();
    ~KspaceImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, int threadId);
    void AfterThreadedGenerateData();

    typename InputImageType::Pointer    m_SpectrumImage;
    typename InputImageType::Pointer    m_FrequencyImage;
    double                              m_tLine;
    double                              m_kOffset;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkKspaceImageFilter.cpp"
#endif

#endif //__itkKspaceImageFilter_h_

