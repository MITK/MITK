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
#ifndef __itkTractsToDWIImageFilter_h__
#define __itkTractsToDWIImageFilter_h__

#include <mitkFiberBundle.h>
#include <itkVnlForwardFFTImageFilter.h>
#include <itkVectorImage.h>
#include <cmath>
#include <mitkFiberfoxParameters.h>
#include <itkTimeProbe.h>
#include <mitkRawShModel.h>
#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>
#include <mitkPointSet.h>
#include <itkLinearInterpolateImageFunction.h>

namespace itk
{

/**
* \brief Generates artificial diffusion weighted image volume from the input fiberbundle using a generic multicompartment model.
* See "Fiberfox: Facilitating the creation of realistic white matter software phantoms" (DOI: 10.1002/mrm.25045) for details.
*/

template< class PixelType >
class TractsToDWIImageFilter : public ImageSource< itk::VectorImage< PixelType, 3 > >
{

public:

    typedef TractsToDWIImageFilter                          Self;
    typedef ImageSource< itk::VectorImage< PixelType, 3 > > Superclass;
    typedef SmartPointer< Self >                            Pointer;
    typedef SmartPointer< const Self >                      ConstPointer;

    typedef typename Superclass::OutputImageType                        OutputImageType;
    typedef itk::Image<double, 4>                                       ItkDoubleImgType4D;
    typedef itk::Image<double, 3>                                       ItkDoubleImgType;
    typedef itk::Image<float, 3>                                        ItkFloatImgType;
    typedef itk::Image<unsigned char, 3>                                ItkUcharImgType;
    typedef mitk::FiberBundle::Pointer                                  FiberBundleType;
    typedef itk::VectorImage< double, 3 >                               DoubleDwiType;
    typedef itk::Matrix<float, 3, 3>                                    MatrixType;
    typedef itk::Image< float, 2 >                                      Float2DImageType;
    typedef itk::Image< vcl_complex< float >, 2 >                       Complex2DImageType;
    typedef itk::Vector< float, 3>                                      VectorType;

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    itkTypeMacro( TractsToDWIImageFilter, ImageSource )

    /** Input */
    itkSetMacro( FiberBundle, FiberBundleType )             ///< Input fiber bundle
    itkSetMacro( InputImage, typename OutputImageType::Pointer )     ///< Input diffusion-weighted image. If no fiber bundle is set, then the acquisition is simulated for this image without a new diffusion simulation.
    itkSetMacro( UseConstantRandSeed, bool )                ///< Seed for random generator.
    void SetParameters( FiberfoxParameters param )  ///< Simulation parameters.
    { m_Parameters = param; }

    /** Output */
    FiberfoxParameters GetParameters(){ return m_Parameters; }
    std::vector< ItkDoubleImgType::Pointer > GetVolumeFractions() ///< one double image for each compartment containing the corresponding volume fraction per voxel
    { return m_VolumeFractions; }
    itkGetMacro( StatusText, std::string )
    itkGetMacro( PhaseImage, DoubleDwiType::Pointer )
    itkGetMacro( KspaceImage, DoubleDwiType::Pointer )
    itkGetMacro( CoilPointset, mitk::PointSet::Pointer )
    itkGetMacro( TickImage, typename Float2DImageType::Pointer )    ///< k-space readout ordering encoded in the voxels
    itkGetMacro( RfImage, typename Float2DImageType::Pointer )    ///< time passed since last RF pulse encoded per voxel

    void GenerateData() override;

    std::vector<DoubleDwiType::Pointer> GetOutputImagesReal() const
    {
      return m_OutputImagesReal;
    }

    std::vector<DoubleDwiType::Pointer> GetOutputImagesImag() const
    {
      return m_OutputImagesImag;
    }

protected:

    TractsToDWIImageFilter();
    ~TractsToDWIImageFilter() override;
    itk::Vector<double, 3> GetItkVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(Vector< float, 3 >& vector);
    double RoundToNearest(double num);
    std::string GetTime();
    bool PrepareLogFile();  /** Prepares the log file and returns true if successful or false if failed. */
    void PrintToLog(std::string m, bool addTime=true, bool linebreak=true, bool stdOut=true);

    /** Transform generated image compartment by compartment, channel by channel and slice by slice using DFT and add k-space artifacts/effects. */
    DoubleDwiType::Pointer SimulateKspaceAcquisition(std::vector< DoubleDwiType::Pointer >& images);

    /** Generate signal of non-fiber compartments. */
    void SimulateExtraAxonalSignal(ItkUcharImgType::IndexType& index, itk::Point<float, 3>& volume_fraction_point, double intraAxonalVolume, int g);

    /** Move fibers to simulate headmotion */
    void SimulateMotion(int g=-1);

    void CheckVolumeFractionImages();
    ItkDoubleImgType::Pointer NormalizeInsideMask(ItkDoubleImgType::Pointer image);
    void InitializeData();
    void InitializeFiberData();

    itk::Point<float, 3> GetMovedPoint(itk::Index<3>& index, bool forward);

    // input
    mitk::FiberfoxParameters                    m_Parameters;
    FiberBundleType                             m_FiberBundle;
    typename OutputImageType::Pointer           m_InputImage;

    // output
    typename OutputImageType::Pointer           m_OutputImage;
    typename DoubleDwiType::Pointer             m_PhaseImage;
    typename DoubleDwiType::Pointer             m_KspaceImage;
    std::vector < typename DoubleDwiType::Pointer > m_OutputImagesReal;
    std::vector < typename DoubleDwiType::Pointer > m_OutputImagesImag;
    std::vector< ItkDoubleImgType::Pointer >    m_VolumeFractions;
    std::string                                 m_StatusText;

    // MISC
    itk::TimeProbe                              m_TimeProbe;
    bool                                        m_UseConstantRandSeed;
    bool                                        m_MaskImageSet;
    ofstream                                    m_Logfile;
    std::string                                 m_MotionLog;
    std::string                                 m_SpikeLog;

    // signal generation
    FiberBundleType                             m_FiberBundleTransformed;   ///< transformed bundle simulating headmotion
    itk::Vector<double,3>                       m_WorkingSpacing;
    itk::Point<double,3>                        m_WorkingOrigin;
    ImageRegion<3>                              m_WorkingImageRegion;
    double                                      m_VoxelVolume;
    std::vector< DoubleDwiType::Pointer >       m_CompartmentImages;
    ItkUcharImgType::Pointer                    m_TransformedMaskImage;     ///< copy of mask image (changes for each motion step)
    ItkUcharImgType::Pointer                    m_UpsampledMaskImage;       ///< helper image for motion simulation
    std::vector< MatrixType >                   m_RotationsInv;
    std::vector< MatrixType >                   m_Rotations;                ///<stores the individual rotation of each volume (needed for k-space simulation to obtain correct frequency map position)
    std::vector< VectorType >                   m_Translations;             ///<stores the individual translation of each volume (needed for k-space simulation to obtain correct frequency map position)
    double                                      m_mmRadius;
    double                                      m_SegmentVolume;
    bool                                        m_UseRelativeNonFiberVolumeFractions;
    mitk::PointSet::Pointer                     m_CoilPointset;
    int                                         m_MotionCounter;
    int                                         m_NumMotionVolumes;

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    itk::LinearInterpolateImageFunction< ItkDoubleImgType, float >::Pointer   m_DoubleInterpolator;
    itk::Vector<double,3>                       m_NullDir;

    Float2DImageType::Pointer                   m_TickImage;
    Float2DImageType::Pointer                   m_RfImage;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToDWIImageFilter.cpp"
#endif

#endif
