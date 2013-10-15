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

// MITK
#include <mitkFiberBundleX.h>
#include <mitkDiffusionSignalModel.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkKspaceArtifact.h>

// ITK
#include <itkImage.h>
#include <itkVectorImage.h>
#include <itkImageSource.h>
#include <itkVnlForwardFFTImageFilter.h>
#include <itkVnlInverseFFTImageFilter.h>

#include <cmath>


namespace itk
{

/**
* \brief Generates artificial diffusion weighted image volume from the input fiberbundle using a generic multicompartment model.   */

template< class PixelType >
class TractsToDWIImageFilter : public ImageSource< itk::VectorImage< PixelType, 3 > >
{

public:

    typedef TractsToDWIImageFilter      Self;
    typedef ImageSource< itk::VectorImage< PixelType, 3 > > Superclass;
    typedef SmartPointer< Self >        Pointer;
    typedef SmartPointer< const Self >  ConstPointer;

    typedef typename Superclass::OutputImageType                     OutputImageType;
    typedef itk::Image<double, 3>                           ItkDoubleImgType;
    typedef itk::Image<float, 3>                            ItkFloatImgType;
    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;
    typedef mitk::FiberBundleX::Pointer                     FiberBundleType;
    typedef itk::VectorImage< double, 3 >                   DoubleDwiType;
    typedef std::vector< mitk::KspaceArtifact<double>* >    KspaceArtifactList;
    typedef std::vector< mitk::DiffusionSignalModel<double>* >    DiffusionModelList;
    typedef itk::Matrix<double, 3, 3>                       MatrixType;
    typedef mitk::DiffusionNoiseModel<double>               NoiseModelType;
    typedef itk::Image< double, 2 >                         SliceType;
    typedef itk::VnlForwardFFTImageFilter<SliceType>::OutputImageType ComplexSliceType;
    typedef itk::Vector< double,3>                      VectorType;
    typedef itk::Point< double,3>                      PointType;

    itkNewMacro(Self)
    itkTypeMacro( TractsToDWIImageFilter, ImageToImageFilter )

    // input
    itkSetMacro( SignalScale, double )
    itkSetMacro( FiberRadius, double )
    itkSetMacro( InterpolationShrink, double )          ///< large values shrink (towards nearest neighbour interpolation), small values strech interpolation function (towards linear interpolation)
    itkSetMacro( VolumeAccuracy, unsigned int )         ///< determines fiber sampling density and thereby the accuracy of the fiber volume fraction
    itkSetMacro( FiberBundle, FiberBundleType )         ///< input fiber bundle
    itkSetMacro( Spacing, VectorType )                  ///< output image spacing
    itkSetMacro( Origin, PointType )                    ///< output image origin
    itkSetMacro( DirectionMatrix, MatrixType )          ///< output image rotation
    itkSetMacro( EnforcePureFiberVoxels, bool )         ///< treat all voxels containing at least one fiber as fiber-only (actually disable non-fiber compartments for this voxel).
    itkSetMacro( ImageRegion, ImageRegion<3> )          ///< output image size
    itkSetMacro( NumberOfRepetitions, unsigned int )    ///< number of acquisition repetitions to reduce noise (default is no additional repetition)
    itkSetMacro( TissueMask, ItkUcharImgType::Pointer ) ///< voxels outside of this binary mask contain only noise (are treated as air)
    void SetNoiseModel(NoiseModelType* noiseModel){ m_NoiseModel = noiseModel; }            ///< generates the noise added to the image values
    void SetFiberModels(DiffusionModelList modelList){ m_FiberModels = modelList; }         ///< generate signal of fiber compartments
    void SetNonFiberModels(DiffusionModelList modelList){ m_NonFiberModels = modelList; }   ///< generate signal of non-fiber compartments
    void SetKspaceArtifacts(KspaceArtifactList artifactList){ m_KspaceArtifacts = artifactList; }
    mitk::LevelWindow GetLevelWindow(){ return m_LevelWindow; }
    itkSetMacro( FrequencyMap, ItkDoubleImgType::Pointer )
    itkSetMacro( kOffset, double )
    itkSetMacro( tLine, double )
    itkSetMacro( tInhom, double )
    itkSetMacro( TE, double )
    itkSetMacro( UseInterpolation, bool )
    itkSetMacro( SimulateEddyCurrents, bool )
    itkSetMacro( SimulateRelaxation, bool )
    itkSetMacro( EddyGradientStrength, double )
    itkSetMacro( AddGibbsRinging, bool )
    itkSetMacro( Spikes, int )
    itkSetMacro( SpikeAmplitude, double )
    itkSetMacro( Wrap, double )

    // output
    std::vector< ItkDoubleImgType::Pointer > GetVolumeFractions(){ return m_VolumeFractions; }

    void GenerateData();

protected:

    TractsToDWIImageFilter();
    virtual ~TractsToDWIImageFilter();
    itk::Point<float, 3> GetItkPoint(double point[3]);
    itk::Vector<double, 3> GetItkVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(Vector< float, 3 >& vector);

    /** Transform generated image compartment by compartment, channel by channel and slice by slice using FFT and add k-space artifacts. */
    DoubleDwiType::Pointer DoKspaceStuff(std::vector< DoubleDwiType::Pointer >& images);

//    /** Rearrange FFT output to shift low frequencies to the iamge center (correct itk). */
//    TractsToDWIImageFilter::ComplexSliceType::Pointer RearrangeSlice(ComplexSliceType::Pointer slice);

    itk::Vector<double,3>               m_Spacing;              ///< output image spacing
    itk::Vector<double,3>               m_UpsampledSpacing;
    itk::Point<double,3>                m_Origin;               ///< output image origin
    MatrixType                          m_DirectionMatrix;      ///< output image rotation
    ImageRegion<3>                      m_ImageRegion;          ///< output image size
    ImageRegion<3>                      m_UpsampledImageRegion;
    ItkUcharImgType::Pointer            m_TissueMask;           ///< voxels outside of this binary mask contain only noise (are treated as air)
    ItkDoubleImgType::Pointer           m_FrequencyMap;         ///< map of the B0 inhomogeneities
    double                              m_kOffset;
    double                              m_tLine;
    double                              m_TE;
    double                              m_tInhom;
    FiberBundleType                     m_FiberBundle;          ///< input fiber bundle
    DiffusionModelList                  m_FiberModels;          ///< generate signal of fiber compartments
    DiffusionModelList                  m_NonFiberModels;       ///< generate signal of non-fiber compartments
    KspaceArtifactList                  m_KspaceArtifacts;
    NoiseModelType*                     m_NoiseModel;           ///< generates the noise added to the image values
    bool                                m_CircleDummy;
    unsigned int                        m_VolumeAccuracy;
    bool                                m_AddGibbsRinging;      ///< causes ringing artifacts
    unsigned int                        m_NumberOfRepetitions;
    bool                                m_EnforcePureFiberVoxels;
    double                              m_InterpolationShrink;
    double                              m_FiberRadius;
    double                              m_SignalScale;
    mitk::LevelWindow                   m_LevelWindow;
    bool                                m_UseInterpolation;
    std::vector< ItkDoubleImgType::Pointer >    m_VolumeFractions;  ///< one double image for each compartment containing the corresponding volume fraction per voxel
    bool                                m_SimulateRelaxation;
    bool                                m_SimulateEddyCurrents;
    double                              m_EddyGradientStrength;
    int                                 m_Spikes;
    double                              m_SpikeAmplitude;
    double                              m_Wrap;
};
}

#include "itkTractsToDWIImageFilter.cpp"

#endif
