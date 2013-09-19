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
#ifndef __itkDftImageFilter_h_
#define __itkDftImageFilter_h_

#include "FiberTrackingExports.h"
#include <itkImageToImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <vcl_complex.h>

namespace itk{

/**
* \brief 2D Discrete Fourier Transform Filter (complex to real). Special issue for Fiberfox -> rearranges slice. */

  template< class TPixelType >
  class DftImageFilter :
      public ImageToImageFilter< Image< vcl_complex< TPixelType > >, Image< TPixelType > >
  {

  public:

    typedef DftImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< vcl_complex< TPixelType > >, Image< TPixelType > > Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(DftImageFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType         InputImageType;
    typedef typename Superclass::OutputImageType        OutputImageType;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;

  protected:
    DftImageFilter();
    ~DftImageFilter() {}

    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDftImageFilter.cpp"
#endif

#endif //__itkDftImageFilter_h_

