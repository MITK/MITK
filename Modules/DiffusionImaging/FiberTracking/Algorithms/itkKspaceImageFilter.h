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
#include <itkImageSource.h>
#include <vcl_complex.h>
#include <vector>

namespace itk{

/**
* \brief Performes deterministic streamline tracking on the input tensor image.   */

  template< class TPixelType >
  class KspaceImageFilter :
      public ImageSource< Image< vcl_complex< TPixelType >, 2 > >
  {

  public:

    typedef KspaceImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageSource< Image< vcl_complex< TPixelType >, 2 > > Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(KspaceImageFilter, ImageToImageFilter);

    typedef typename itk::Image< double, 2 >                InputImageType;
    typedef typename InputImageType::Pointer                InputImagePointerType;
    typedef typename Superclass::OutputImageType            OutputImageType;
    typedef typename Superclass::OutputImageRegionType      OutputImageRegionType;

    itkSetMacro( FrequencyMap, typename InputImageType::Pointer );
    itkSetMacro( tLine, double );
    itkSetMacro( kOffset, double );
    itkSetMacro( TE, double);
    itkSetMacro( Tinhom, double);
    itkSetMacro( SimulateRelaxation, bool );
    void SetT2( std::vector< double > t2Vector ) { m_T2=t2Vector; }
    void SetCompartmentImages( std::vector< InputImagePointerType > cImgs ) { m_CompartmentImages=cImgs; }

  protected:
    KspaceImageFilter();
    ~KspaceImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);
    void AfterThreadedGenerateData();

    typename InputImageType::Pointer        m_FrequencyMap;
    double                                  m_tLine;
    double                                  m_kOffset;

    double                                  m_Tinhom;
    double                                  m_TE;
    std::vector< double >                   m_T2;
    std::vector< InputImagePointerType >    m_CompartmentImages;
    bool                                    m_SimulateRelaxation;
    bool                                    m_SimulateDistortions;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkKspaceImageFilter.cpp"
#endif

#endif //__itkKspaceImageFilter_h_

