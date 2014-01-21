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
#include <itkMersenneTwisterRandomVariateGenerator.h>

using namespace std;

namespace itk{

/**
* \brief Simulates k-space acquisition of one slice with a single shot EPI sequence. Enables the simulation of various effects occuring during real MR acquisitions:
* - T2 signal relaxation
* - Spikes
* - N/2 Ghosts
* - Aliasing (wrap around)
* - Image distortions (off-frequency effects)
* - Gibbs ringing
* - Eddy current effects
* Based on a discrete fourier transformation.
* See "Fiberfox: Facilitating the creation of realistic white matter software phantoms" (DOI: 10.1002/mrm.25045) for details.
*/

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

    itkSetMacro( FrequencyMap, typename InputImageType::Pointer )   ///< Used to simulate distortions. Specifies additional frequency component per voxel.
    itkSetMacro( tLine, double )                    ///< Time needed to fill one line in k-space (in ms).
    itkSetMacro( kOffset, double )                  ///< Causes N/2 ghosting artifacts.
    itkSetMacro( TE, double)                        ///< Echo time TE (in ms).
    itkSetMacro( Tinhom, double)                    ///< T2' signal relaxation constant (in ms).
    itkSetMacro( Tau, double)                       ///< Eddy current decay constant (in ms).
    itkSetMacro( SimulateRelaxation, bool )         ///< Enable T2 signal relaxation.
    itkSetMacro( Z, double )                        ///< Slice position, necessary for eddy current simulation.
    itkSetMacro( DirectionMatrix, MatrixType )      ///< Image rotation matrix, necessary for eddy current simulation.
    itkSetMacro( SignalScale, double )              ///< Scaling factor for resulting signal.
    itkSetMacro( OutSize, itk::Size<2> )            ///< Output slice size. Can be different from input size, e.g. if Gibbs ringing is enabled.
    itkSetMacro( Spikes, int )                      ///< Number of randomly placed spikes per slice.
    itkSetMacro( SpikeAmplitude, double )           ///< Spike amplitude relative to the largest slice value (magnitude of complex).
    itkSetMacro( UseConstantRandSeed, bool )        ///< Use constant seed for random generator for reproducible results.
    itkSetMacro( EddyGradientMagnitude, double)     ///< Magnitude of eddy current induced gradients in T/m

    void SetCompartmentImages( std::vector< InputImagePointerType > cImgs ) { m_CompartmentImages=cImgs; }  ///< One signal image per compartment.
    void SetT2( std::vector< double > t2Vector ) { m_T2=t2Vector; } ///< One T2 relaxation constant per compartment image.
    void SetDiffusionGradientDirection(itk::Vector<double,3> g) { m_DiffusionGradientDirection=g; } ///< Gradient direction is needed for eddy current simulation.

  protected:
    KspaceImageFilter();
    ~KspaceImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);
    void AfterThreadedGenerateData();

    bool                                    m_SimulateRelaxation;
    typename InputImageType::Pointer        m_FrequencyMap;
    double                                  m_tLine;
    double                                  m_kOffset;
    double                                  m_Tinhom;
    double                                  m_TE;
    vector< double >                        m_T2;
    vector< InputImagePointerType >         m_CompartmentImages;
    itk::Vector<double,3>                   m_DiffusionGradientDirection;
    double                                  m_Tau;
    double                                  m_EddyGradientMagnitude;
    double                                  m_Z;
    MatrixType                              m_DirectionMatrix;
    bool                                    m_IsBaseline;
    double                                  m_SignalScale;
    itk::Size<2>                            m_OutSize;
    int                                     m_Spikes;
    double                                  m_SpikeAmplitude;
    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    bool                                    m_UseConstantRandSeed;
    vcl_complex<double>                     m_Spike;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkKspaceImageFilter.cpp"
#endif

#endif //__itkKspaceImageFilter_h_

