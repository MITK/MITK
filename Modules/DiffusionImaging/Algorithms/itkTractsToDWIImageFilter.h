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

// ITK
#include <itkImage.h>
#include <itkVectorImage.h>
#include <itkImageSource.h>
#include <itkFFTRealToComplexConjugateImageFilter.h>
#include <itkFFTComplexConjugateToRealImageFilter.h>

typedef itk::VectorImage< short, 3 > DWIImageType;

namespace itk
{

/**
* \brief Generates artificial diffusion weighted image volume from the input fiberbundle using a generic multicompartment model.   */

class TractsToDWIImageFilter : public ImageSource< DWIImageType >
{

public:
    typedef TractsToDWIImageFilter      Self;
    typedef ImageSource< DWIImageType > Superclass;
    typedef SmartPointer< Self >        Pointer;
    typedef SmartPointer< const Self >  ConstPointer;

    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;
    typedef mitk::FiberBundleX::Pointer                     FiberBundleType;
    typedef itk::VectorImage< double, 3 >                   DoubleDwiType;
    typedef std::vector< DiffusionSignalModel<double>* >    DiffusionModelList;
    typedef itk::Matrix<double, 3, 3>                       MatrixType;
    typedef mitk::DiffusionNoiseModel<double>               NoiseModelType;
    typedef itk::Image< double, 2 >                         SliceType;
    typedef itk::FFTRealToComplexConjugateImageFilter< double, 2 >::OutputImageType ComplexSliceType;

    itkNewMacro(Self)
    itkTypeMacro( TractsToDWIImageFilter, ImageToImageFilter )

    // input
    itkSetMacro( KspaceCropping, double )               ///< k-space cropping factor
    itkSetMacro( OuputKspaceImage, bool )               ///< output image of the k-space instead of the dwi (inverse fourier transformed k-space)
    itkSetMacro( AddGibbsRinging, bool )                ///< add Gibbs ringing artifact
    itkSetMacro( AddT2Smearing, bool )                  ///< add T2 induced k-space smearing artifact
    itkSetMacro( ReadoutPulseLength, unsigned int )     ///< time needed to read one k-space row. parameter for artificial smearing artifacts.
    itkSetMacro( FiberBundle, FiberBundleType )         ///< input fiber bundle
    itkSetMacro( Spacing, mitk::Vector3D )              ///< output image spacing
    itkSetMacro( Origin, mitk::Point3D )                ///< output image origin
    itkSetMacro( DirectionMatrix, MatrixType )          ///< output image rotation
    itkSetMacro( ImageRegion, ImageRegion<3> )          ///< output image size
    itkSetMacro( TissueMask, ItkUcharImgType::Pointer ) ///< voxels outside of this binary mask contain only noise (are treated as air)
    void SetNoiseModel(NoiseModelType* noiseModel){ m_NoiseModel = noiseModel; }            ///< generates the noise added to the image values
    void SetFiberModels(DiffusionModelList modelList){ m_FiberModels = modelList; }         ///< generate signal of fiber compartments
    void SetNonFiberModels(DiffusionModelList modelList){ m_NonFiberModels = modelList; }   ///< generate signal of non-fiber compartments

    void GenerateData();

protected:

    TractsToDWIImageFilter();
    virtual ~TractsToDWIImageFilter();
    itk::Point<float, 3> GetItkPoint(double point[3]);
    itk::Vector<double, 3> GetItkVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(Vector< float, 3 >& vector);

    /** Transform generated image compartment by compartment, channel by channel and slice by slice using FFT and add k-space artifacts. */
    std::vector< DoubleDwiType::Pointer > AddKspaceArtifacts(std::vector< DoubleDwiType::Pointer >& images);

    /** Crop k space to simulate undersampling. Introduces artificial Gibbs ringing artifacts. */
    ComplexSliceType::Pointer CropSlice(ComplexSliceType::Pointer image);

    /** Simulate T2 signal decay during k-space readout. Adds smearing artifact. */
    void AddT2Smearing(ComplexSliceType::Pointer slice, double T2);

    /** Rearrange FFT output to shift low frequencies to the iamge center (correct itk). */
    TractsToDWIImageFilter::ComplexSliceType::Pointer RearrangeSlice(ComplexSliceType::Pointer slice);

    mitk::Vector3D                      m_Spacing;              ///< output image spacing
    mitk::Point3D                       m_Origin;               ///< output image origin
    MatrixType                          m_DirectionMatrix;      ///< output image rotation
    ImageRegion<3>                      m_ImageRegion;          ///< output image size
    ItkUcharImgType::Pointer            m_TissueMask;           ///< voxels outside of this binary mask contain only noise (are treated as air)
    FiberBundleType                     m_FiberBundle;          ///< input fiber bundle
    DiffusionModelList                  m_FiberModels;          ///< generate signal of fiber compartments
    DiffusionModelList                  m_NonFiberModels;       ///< generate signal of non-fiber compartments
    NoiseModelType*                     m_NoiseModel;           ///< generates the noise added to the image values
    double                              m_KspaceCropping;       ///< cropping of k-space (introduces gibbs ringing artifacts)
    unsigned int                        m_ReadoutPulseLength;   ///< time needed to read one k-space row. parameter for artificial smearing artifacts.
    bool                                m_AddT2Smearing;        ///< add T2 smearing artifact or not
    bool                                m_AddGibbsRinging;      ///< add Gibbs ringing artifact or not
    bool                                m_OuputKspaceImage;
};
}

#include "itkTractsToDWIImageFilter.cpp"

#endif
