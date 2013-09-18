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
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(KspaceImageFilter, ImageToImageFilter)

    typedef typename itk::Image< double, 2 >                InputImageType;
    typedef typename InputImageType::Pointer                InputImagePointerType;
    typedef typename Superclass::OutputImageType            OutputImageType;
    typedef typename Superclass::OutputImageRegionType      OutputImageRegionType;
    typedef itk::Matrix<double, 3, 3>                       MatrixType;
    typedef itk::Point<double,2>                            Point2D;

    itkSetMacro( FrequencyMap, typename InputImageType::Pointer )
    itkSetMacro( tLine, double )
    itkSetMacro( kOffset, double )
    itkSetMacro( TE, double)
    itkSetMacro( Tinhom, double)
    itkSetMacro( Tau, double)
    itkSetMacro( SimulateRelaxation, bool )
    itkSetMacro( SimulateEddyCurrents, bool )
    itkSetMacro( Z, double )
    itkSetMacro( DirectionMatrix, MatrixType )
    itkSetMacro( SignalScale, double )
    itkSetMacro( OutSize, itk::Size<2> )
    itkSetMacro( Spikes, int )
    itkSetMacro( SpikeAmplitude, double )

    void SetT2( std::vector< double > t2Vector ) { m_T2=t2Vector; }
    void SetCompartmentImages( std::vector< InputImagePointerType > cImgs ) { m_CompartmentImages=cImgs; }
    void SetDiffusionGradientDirection(itk::Vector<double,3> g) { m_DiffusionGradientDirection=g; }
    void SetEddyGradientMagnitude(double g_mag) { m_EddyGradientMagnitude=g_mag; }    ///< in T/m

  protected:
    KspaceImageFilter();
    ~KspaceImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);
    void AfterThreadedGenerateData();

    bool                                    m_SimulateRelaxation;
    bool                                    m_SimulateDistortions;
    bool                                    m_SimulateEddyCurrents;

    typename InputImageType::Pointer        m_TEMPIMAGE;
    typename InputImageType::Pointer        m_FrequencyMap;
    double                                  m_tLine;
    double                                  m_kOffset;

    double                                  m_Tinhom;
    double                                  m_TE;
    std::vector< double >                   m_T2;
    std::vector< InputImagePointerType >    m_CompartmentImages;
    itk::Vector<double,3>                   m_DiffusionGradientDirection;
    double                                  m_Tau;                          ///< eddy current decay constant
    double                                  m_EddyGradientMagnitude;   ///< in T/m
    double                                  m_Z;
    MatrixType                              m_DirectionMatrix;
    bool                                    m_IsBaseline;
    double                                  m_SignalScale;
    itk::Size<2>                            m_OutSize;
    int                                     m_Spikes;
    double                                  m_SpikeAmplitude;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkKspaceImageFilter.cpp"
#endif

#endif //__itkKspaceImageFilter_h_

