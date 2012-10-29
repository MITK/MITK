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
/** \class DwiPhantomGenerationFilter
 */

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
    itkSetMacro( BValue, float )
    itkSetMacro( SignalScale, float )
    itkSetMacro( NoiseVariance, double )
    itkSetMacro( GreyMatterAdc, float )
    itkSetMacro( Spacing, mitk::Vector3D )
    itkSetMacro( Origin, mitk::Point3D )
    itkSetMacro( DirectionMatrix, MatrixType )
    itkSetMacro( ImageRegion, ImageRegion<3> )
    itkSetMacro( SimulateBaseline, bool )

    // output
    itkGetMacro( DirectionImageContainer, ItkDirectionImageContainer::Pointer)
    itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)
    itkGetMacro( SNRImage, ItkFloatImgType::Pointer)
    itkGetMacro( OutputFiberBundle, mitk::FiberBundleX::Pointer)

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
    ItkDirectionImageContainer::Pointer             m_DirectionImageContainer;
    ItkUcharImgType::Pointer                        m_NumDirectionsImage;
    ItkFloatImgType::Pointer                        m_SNRImage;
    mitk::FiberBundleX::Pointer                     m_OutputFiberBundle;

    // signal regions
    std::vector< ItkUcharImgType::Pointer >         m_SignalRegions;
    std::vector< float >                            m_TensorFA;
    std::vector< float >                            m_TensorADC;
    std::vector< float >                            m_TensorWeight;
    std::vector< vnl_vector_fixed<double, 3> >      m_TensorDirection;

    // signal related variable
    GradientListType                                m_GradientList;
    std::vector< itk::DiffusionTensor3D<float> >    m_TensorList;
    float                                           m_BValue;
    float                                           m_SignalScale;
    Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    int                                             m_BaselineImages;
    double                                          m_MaxBaseline;
    double                                          m_MeanBaseline;
    double                                          m_NoiseVariance;
    float                                           m_GreyMatterAdc;
    bool                                            m_SimulateBaseline;
    TOutputScalarType                               m_DefaultBaseline;

    double GetTensorL2Norm(itk::DiffusionTensor3D<float>& T);
    void GenerateTensors();
    typename OutputImageType::PixelType SimulateMeasurement(itk::DiffusionTensor3D<float>& tensor, float weight);
    void AddNoise(typename OutputImageType::PixelType& pix);
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDwiPhantomGenerationFilter.cpp"
#endif

#endif //__itkDwiPhantomGenerationFilter_h_

