/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDiffusionTensor3DReconstructionImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006-03-27 17:01:06 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDwiPhantomGenerationFilter_h_
#define __itkDwiPhantomGenerationFilter_h_

#include <itkImageSource.h>
#include <itkVectorImage.h>
#include <itkDiffusionTensor3D.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <mitkFiberBundleX.h>

namespace itk{

/**
* \brief Generation of synthetic diffusion weighted images using a second rank tensor model.   */

template< class TOutputScalarType >
class DwiPhantomGenerationFilter : public ImageSource< itk::VectorImage<TOutputScalarType,3> >
{

public:

    typedef DwiPhantomGenerationFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageSource< itk::VectorImage<TOutputScalarType,3> > Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(DwiPhantomGenerationFilter, ImageSource)

    typedef typename Superclass::OutputImageType        OutputImageType;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
    typedef itk::VectorContainer< int, double >         AnglesContainerType;
    typedef Vector<double,3>                            GradientType;
    typedef std::vector<GradientType>                   GradientListType;
    typedef itk::Matrix<double, 3, 3>                   MatrixType;
    typedef itk::Image<unsigned char, 3>                ItkUcharImgType;
    typedef itk::Image<float, 3>                        ItkFloatImgType;

    typedef Image< Vector< float, 3 >, 3>                                 ItkDirectionImage;
    typedef VectorContainer< unsigned int, ItkDirectionImage::Pointer >   ItkDirectionImageContainer;

    void SetGradientList(GradientListType gradientList){m_GradientList = gradientList;}
    void SetSignalRegions(std::vector< ItkUcharImgType::Pointer > signalRegions){m_SignalRegions = signalRegions;}
    void SetTensorFA(std::vector< float > faList){m_TensorFA = faList;}
    void SetTensorADC(std::vector< float > adcList){m_TensorADC = adcList;}
    void SetTensorWeight(std::vector< float > weightList){m_TensorWeight = weightList;}
    void SetTensorDirection(std::vector< vnl_vector_fixed<double, 3> > directionList){m_TensorDirection = directionList;}

    // input parameters
    itkSetMacro( BValue, float )                ///< signal parameter
    itkSetMacro( SignalScale, float )           ///< scaling factor for signal
    itkSetMacro( NoiseVariance, double )        ///< variance of rician noise
    itkSetMacro( GreyMatterAdc, float )         ///< ADC of isotropic diffusion tensor
    itkSetMacro( Spacing, mitk::Vector3D )      ///< parameter of output image
    itkSetMacro( Origin, mitk::Point3D )        ///< parameter of output image
    itkSetMacro( DirectionMatrix, MatrixType )  ///< parameter of output image
    itkSetMacro( ImageRegion, ImageRegion<3> )  ///< parameter of output image
    itkSetMacro( SimulateBaseline, bool )       ///< generate baseline image values as the l2 norm of the corresponding tensor used for the diffusion signal generation

    // output
    itkGetMacro( DirectionImageContainer, ItkDirectionImageContainer::Pointer)  ///< contains one vectorimage for each input ROI
    itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)                  ///< contains number of directions per voxel
    itkGetMacro( SNRImage, ItkFloatImgType::Pointer)                            ///< contains local SNR values
    itkGetMacro( OutputFiberBundle, mitk::FiberBundleX::Pointer)                ///< output vector field

    protected:
    DwiPhantomGenerationFilter();
    ~DwiPhantomGenerationFilter(){}

    void GenerateData();

private:

    // image variables
    mitk::Vector3D                                  m_Spacing;
    mitk::Point3D                                   m_Origin;
    itk::Matrix<double, 3, 3>                       m_DirectionMatrix;
    ImageRegion<3>                                  m_ImageRegion;
    ItkDirectionImageContainer::Pointer             m_DirectionImageContainer;  ///< contains one vectorimage for each input ROI
    ItkUcharImgType::Pointer                        m_NumDirectionsImage;       ///< contains number of directions per voxel
    ItkFloatImgType::Pointer                        m_SNRImage;                 ///< contains local SNR values
    mitk::FiberBundleX::Pointer                     m_OutputFiberBundle;        ///< output vector field

    // signal regions
    std::vector< ItkUcharImgType::Pointer >         m_SignalRegions;    ///< binary images defining the regions for the signal generation
    std::vector< float >                            m_TensorFA;         ///< kernel tensor parameter
    std::vector< float >                            m_TensorADC;        ///< kernel tensor parameter
    std::vector< float >                            m_TensorWeight;     ///< weight factor of the signal
    std::vector< vnl_vector_fixed<double, 3> >      m_TensorDirection;  ///< principal direction of the kernel tensor in the different ROIs

    // signal related variable
    GradientListType                                m_GradientList;     ///< list of used diffusion gradient directions
    std::vector< itk::DiffusionTensor3D<float> >    m_TensorList;       ///< kernel tensor rotated in the different directions
    float                                           m_BValue;           ///< signal parameter
    float                                           m_SignalScale;      ///< scaling factor for signal
    Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    int                                             m_BaselineImages;   ///< number of simulated baseline images
    double                                          m_MaxBaseline;      ///< maximum value of the baseline image
    double                                          m_MeanBaseline;     ///< mean value of the baseline image
    double                                          m_NoiseVariance;    ///< variance of rician noise
    float                                           m_GreyMatterAdc;    ///< ADC of isotropic diffusion tensor
    bool                                            m_SimulateBaseline; ///< generate baseline image values as the l2 norm of the corresponding tensor used for the diffusion signal generation
    TOutputScalarType                               m_DefaultBaseline;  ///< default value for baseline image

    double GetTensorL2Norm(itk::DiffusionTensor3D<float>& T);
    void GenerateTensors();     ///< rotates kernel tensor in the direction of the input direction vectors
    typename OutputImageType::PixelType SimulateMeasurement(itk::DiffusionTensor3D<float>& tensor, float weight);
    void AddNoise(typename OutputImageType::PixelType& pix);    ///< Adds rician noise to the input pixel
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDwiPhantomGenerationFilter.cpp"
#endif

#endif //__itkDwiPhantomGenerationFilter_h_

