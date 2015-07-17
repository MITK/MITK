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

#include <MitkFiberTrackingExports.h>
#include <itkImageSource.h>
#include <vcl_complex.h>
#include <vector>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <mitkFiberfoxParameters.h>
#include <mitkFiberBundle.h>
#include <mitkAcquisitionType.h>

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
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(KspaceImageFilter, ImageToImageFilter)

    typedef typename itk::Image< double, 2 >                InputImageType;
    typedef typename InputImageType::Pointer                InputImagePointerType;
    typedef typename Superclass::OutputImageType            OutputImageType;
    typedef typename Superclass::OutputImageRegionType      OutputImageRegionType;
    typedef itk::Matrix<double, 3, 3>                       MatrixType;
    typedef itk::Point<double,2>                            Point2D;
    typedef itk::Vector< double,3>                          DoubleVectorType;
    typedef itk::Image<double, 3>                           ItkDoubleImgType;

    itkSetMacro( SpikesPerSlice, unsigned int )     ///< Number of spikes per slice. Corresponding parameter in fiberfox parameter object specifies the number of spikes for the whole image and can thus not be used here.
    itkSetMacro( Z, double )                        ///< Slice position, necessary for eddy current simulation.
    itkSetMacro( UseConstantRandSeed, bool )        ///< Use constant seed for random generator for reproducible results. ONLY USE FOR TESTING PURPOSES!
    itkSetMacro( Rotation, DoubleVectorType )
    itkSetMacro( Translation, DoubleVectorType )
    itkSetMacro( Zidx, int )
    itkSetMacro( FiberBundle, FiberBundle::Pointer )
    itkSetMacro( CoilPosition, DoubleVectorType )
    itkGetMacro( KSpaceImage, typename InputImageType::Pointer )    ///< k-space magnitude image
    itkGetMacro( SpikeLog, std::string )

    void SetParameters( FiberfoxParameters<double>* param ){ m_Parameters = param; }

    void SetCompartmentImages( std::vector< InputImagePointerType > cImgs ) { m_CompartmentImages=cImgs; }  ///< One signal image per compartment.
    void SetT2( std::vector< double > t2Vector ) { m_T2=t2Vector; } ///< One T2 relaxation constant per compartment image.
    void SetT1( std::vector< double > t1Vector ) { m_T1=t1Vector; } ///< One T1 relaxation constant per compartment image.
    void SetDiffusionGradientDirection(itk::Vector<double,3> g) { m_DiffusionGradientDirection=g; } ///< Gradient direction is needed for eddy current simulation.

  protected:
    KspaceImageFilter();
    ~KspaceImageFilter() {}

    double CoilSensitivity(DoubleVectorType& pos);

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadID);
    void AfterThreadedGenerateData();
    double InterpolateFmapValue(itk::Point<float, 3> itkP);

    DoubleVectorType                        m_CoilPosition;
    FiberfoxParameters<double>*             m_Parameters;
    vector< double >                        m_T2;
    vector< double >                        m_T1;
    vector< InputImagePointerType >         m_CompartmentImages;
    itk::Vector<double,3>                   m_DiffusionGradientDirection;
    double                                  m_Z;
    int                                     m_Zidx;
    bool                                    m_UseConstantRandSeed;
    unsigned int                            m_SpikesPerSlice;
    FiberBundle::Pointer                    m_FiberBundle;
    double                                  m_Gamma;
    DoubleVectorType                        m_Rotation;     ///< used to find correct point in frequency map (head motion)
    DoubleVectorType                        m_Translation;  ///< used to find correct point in frequency map (head motion)

    bool                                    m_IsBaseline;
    vcl_complex<double>                     m_Spike;
    MatrixType                              m_Transform;
    std::string                             m_SpikeLog;

    double                                  m_CoilSensitivityFactor;
    typename InputImageType::Pointer        m_KSpaceImage;
    typename InputImageType::Pointer        m_TimeFromEchoImage;
    typename InputImageType::Pointer        m_ReadoutTimeImage;
    AcquisitionType*                        m_ReadoutScheme;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkKspaceImageFilter.cpp"
#endif

#endif //__itkKspaceImageFilter_h_

