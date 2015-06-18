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
    typedef itk::VectorImage< double, 4 >                               DoubleDwiType4D;
    typedef itk::Matrix<double, 3, 3>                                   MatrixType;
    typedef itk::Image< double, 2 >                                     SliceType;
    typedef itk::VnlForwardFFTImageFilter<SliceType>::OutputImageType   ComplexSliceType;
    typedef itk::VectorImage< vcl_complex< double >, 3 >                ComplexDwiType;
    typedef itk::Vector< double,3>                                      DoubleVectorType;

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    itkTypeMacro( TractsToDWIImageFilter, ImageSource )

    /** Input */
    itkSetMacro( FiberBundle, FiberBundleType )             ///< Input fiber bundle
    itkSetMacro( UseConstantRandSeed, bool )                ///< Seed for random generator.
    void SetParameters( FiberfoxParameters<double> param )  ///< Simulation parameters.
    { m_Parameters = param; }

    /** Output */
    FiberfoxParameters<double> GetParameters(){ return m_Parameters; }
    std::vector< ItkDoubleImgType::Pointer > GetVolumeFractions() ///< one double image for each compartment containing the corresponding volume fraction per voxel
    { return m_VolumeFractions; }
    mitk::LevelWindow GetLevelWindow(){ return m_LevelWindow; }
    itkGetMacro( StatusText, std::string )
    itkGetMacro( PhaseImage, DoubleDwiType::Pointer )
    itkGetMacro( KspaceImage, DoubleDwiType::Pointer )
    itkGetMacro( CoilPointset, mitk::PointSet::Pointer )

    void GenerateData();

protected:

    TractsToDWIImageFilter();
    virtual ~TractsToDWIImageFilter();
    itk::Point<float, 3> GetItkPoint(double point[3]);
    itk::Vector<double, 3> GetItkVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(Vector< float, 3 >& vector);
    double RoundToNearest(double num);
    std::string GetTime();

    /** Transform generated image compartment by compartment, channel by channel and slice by slice using DFT and add k-space artifacts. */
    DoubleDwiType::Pointer SimulateKspaceAcquisition(std::vector< DoubleDwiType::Pointer >& images);

    /** Generate signal of non-fiber compartments. */
    void SimulateExtraAxonalSignal(ItkUcharImgType::IndexType index, double intraAxonalVolume, int g=-1);

    /** Move fibers to simulate headmotion */
    void SimulateMotion(int g=-1);

    void CheckVolumeFractionImages();
    ItkDoubleImgType::Pointer NormalizeInsideMask(ItkDoubleImgType::Pointer image);
    void InitializeData();
    void InitializeFiberData();
    double InterpolateValue(itk::Point<float, 3> itkP, ItkDoubleImgType::Pointer img);

    // input
    mitk::FiberfoxParameters<double>            m_Parameters;
    FiberBundleType                             m_FiberBundle;

    // output
    typename OutputImageType::Pointer           m_OutputImage;
    typename DoubleDwiType::Pointer             m_PhaseImage;
    typename DoubleDwiType::Pointer             m_KspaceImage;
    mitk::LevelWindow                           m_LevelWindow;
    std::vector< ItkDoubleImgType::Pointer >    m_VolumeFractions;
    std::string                                 m_StatusText;

    // MISC
    itk::TimeProbe                              m_TimeProbe;
    bool                                        m_UseConstantRandSeed;
    bool                                        m_MaskImageSet;
    ofstream                                    m_MotionLogfile;

    // signal generation
    FiberBundleType                             m_FiberBundleWorkingCopy;   ///< we work on an upsampled version of the input bundle
    FiberBundleType                             m_FiberBundleTransformed;   ///< transformed bundle simulating headmotion
    itk::Vector<double,3>                       m_WorkingSpacing;
    itk::Point<double,3>                        m_WorkingOrigin;
    ImageRegion<3>                              m_WorkingImageRegion;
    double                                      m_VoxelVolume;
    std::vector< DoubleDwiType::Pointer >       m_CompartmentImages;
    ItkUcharImgType::Pointer                    m_TransformedMaskImage;     ///< copy of mask image (changes for each motion step)
    ItkUcharImgType::Pointer                    m_UpsampledMaskImage;       ///< helper image for motion simulation
    DoubleVectorType                            m_Rotation;
    DoubleVectorType                            m_Translation;
    std::vector< DoubleVectorType >             m_Rotations;                ///<stores the individual rotation of each volume (needed for k-space simulation to obtain correct frequency map position)
    std::vector< DoubleVectorType >             m_Translations;             ///<stores the individual translation of each volume (needed for k-space simulation to obtain correct frequency map position)
    double                                      m_mmRadius;
    double                                      m_SegmentVolume;
    bool                                        m_UseRelativeNonFiberVolumeFractions;
    mitk::PointSet::Pointer                     m_CoilPointset;
    int                                         m_MotionCounter;
    int                                         m_NumMotionVolumes;

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToDWIImageFilter.cpp"
#endif

#endif
