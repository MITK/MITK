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

  template< class ScalarType >
  class KspaceImageFilter :
      public ImageSource< Image< vcl_complex< ScalarType >, 2 > >
  {

  public:

    typedef KspaceImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageSource< Image< vcl_complex< ScalarType >, 2 > > Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(KspaceImageFilter, ImageToImageFilter)

    typedef typename itk::Image< ScalarType, 2 >            InputImageType;
    typedef typename InputImageType::Pointer                InputImagePointerType;
    typedef typename Superclass::OutputImageType            OutputImageType;
    typedef typename Superclass::OutputImageRegionType      OutputImageRegionType;
    typedef itk::Matrix<float, 3, 3>                        MatrixType;
    typedef itk::Point<float,2>                             Point2D;
    typedef itk::Vector< float,3>                           VectorType;

    itkSetMacro( SpikesPerSlice, unsigned int )     ///< Number of spikes per slice. Corresponding parameter in fiberfox parameter object specifies the number of spikes for the whole image and can thus not be used here.
    itkSetMacro( Z, double )                        ///< Slice position, necessary for eddy current simulation.
    itkSetMacro( RandSeed, int )                  ///< Use constant seed for random generator for reproducible results.
    itkSetMacro( Translation, VectorType )
    itkSetMacro( RotationMatrix, MatrixType )
    itkSetMacro( Zidx, int )
    itkSetMacro( StoreTimings, bool )
    itkSetMacro( FiberBundle, FiberBundle::Pointer )
    itkSetMacro( CoilPosition, VectorType )
    itkGetMacro( KSpaceImage, typename InputImageType::Pointer )    ///< k-space magnitude image
    itkGetMacro( TickImage, typename InputImageType::Pointer )    ///< k-space readout ordering encoded in the voxels
    itkGetMacro( RfImage, typename InputImageType::Pointer )    ///< time passed since last RF pulse encoded per voxel
    itkGetMacro( SpikeLog, std::string )

    void SetParameters( FiberfoxParameters* param ){ m_Parameters = param; }

    void SetCompartmentImages( std::vector< InputImagePointerType > cImgs ) { m_CompartmentImages=cImgs; }  ///< One signal image per compartment.
    void SetT2( std::vector< float > t2Vector ) { m_T2=t2Vector; } ///< One T2 relaxation constant per compartment image.
    void SetT1( std::vector< float > t1Vector ) { m_T1=t1Vector; } ///< One T1 relaxation constant per compartment image.
    void SetDiffusionGradientDirection(itk::Vector<double,3> g) { m_DiffusionGradientDirection=g; } ///< Gradient direction is needed for eddy current simulation.

  protected:
    KspaceImageFilter();
    ~KspaceImageFilter() override {}

    float CoilSensitivity(VectorType& pos);

    void BeforeThreadedGenerateData() override;
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadID) override;
    void AfterThreadedGenerateData() override;

    VectorType                              m_CoilPosition;
    FiberfoxParameters*                     m_Parameters;
    std::vector< float >                    m_T2;
    std::vector< float >                    m_T1;
    std::vector< float >                    m_T1Relax;
    std::vector< InputImagePointerType >    m_CompartmentImages;
    itk::Vector<double,3>                   m_DiffusionGradientDirection;
    float                                   m_Z;
    int                                     m_Zidx;
    int                                     m_RandSeed;
    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    unsigned int                            m_SpikesPerSlice;
    FiberBundle::Pointer                    m_FiberBundle;
    float                                   m_Gamma;
    VectorType                              m_Translation;  ///< used to find correct point in frequency map (head motion)
    MatrixType                              m_RotationMatrix;
    float                                   m_TransX;
    float                                   m_TransY;
    float                                   m_TransZ;

    bool                                    m_IsBaseline;
    vcl_complex<ScalarType>                 m_Spike;
    MatrixType                              m_Transform;
    std::string                             m_SpikeLog;

    float                                   m_CoilSensitivityFactor;
    typename InputImageType::Pointer        m_KSpaceImage;
    typename InputImageType::Pointer        m_TickImage;
    typename InputImageType::Pointer        m_RfImage;
    AcquisitionType*                        m_ReadoutScheme;

    typename itk::Image< ScalarType, 2 >::Pointer m_MovedFmap;
    int                                     ringing_lines_x;
    int                                     ringing_lines_y;
    float                                   kxMax;
    float                                   kyMax;
    float                                   xMax;
    float                                   yMax;
    float                                   yMaxFov;
    float                                   yMaxFov_half;
    float                                   numPix;
    bool                                    m_StoreTimings;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkKspaceImageFilter.cpp"
#endif

#endif //__itkKspaceImageFilter_h_

