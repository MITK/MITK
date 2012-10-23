/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTractsToDWIImageFilter_h__
#define __itkTractsToDWIImageFilter_h__

// MITK
#include <mitkFiberBundleX.h>

// ITK
#include <itkImage.h>
#include <itkProcessObject.h>
#include <itkImageSource.h>

typedef itk::VectorImage< short, 3 > DWIImageType;

namespace itk
{
class TractsToDWIImageFilter : public ImageSource< DWIImageType >
{

public:
    typedef TractsToDWIImageFilter      Self;
    typedef ImageSource< DWIImageType > Superclass;
    typedef SmartPointer< Self >        Pointer;
    typedef SmartPointer< const Self >  ConstPointer;

    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;
    typedef itk::Image<float, 3>                            ItkFloatImgType;
    typedef itk::DiffusionTensor3D< float >                 ItkTensorType;
    typedef itk::Image<ItkTensorType,3>                     ItkTensorImageType;
    typedef mitk::FiberBundleX::Pointer                     FiberBundleType;
    typedef Vector<double,3>                                GradientType;
    typedef std::vector<GradientType>                       GradientListType;
    typedef itk::Matrix<double, 3, 3>                       MatrixType;

    itkNewMacro(Self)
    itkTypeMacro( TractsToDWIImageFilter, ImageToImageFilter )

    // input
    itkSetMacro(FiberBundle, FiberBundleType)
    void SetGradientList(GradientListType gradientList) {m_GradientList = gradientList; }


//    itkSetMacro( BValue, float )
//    itkSetMacro( SignalScale, float )
//    itkSetMacro( NoiseVariance, double )
//    itkSetMacro( GreyMatterAdc, float )
    itkSetMacro( Spacing, mitk::Vector3D )
    itkSetMacro( Origin, mitk::Point3D )
    itkSetMacro( DirectionMatrix, MatrixType )
    itkSetMacro( ImageRegion, ImageRegion<3> )

    // output
    itkGetMacro(TensorImage, ItkTensorImageType::Pointer)

    void GenerateData();

protected:

    TractsToDWIImageFilter();
    virtual ~TractsToDWIImageFilter();
    itk::Point<float, 3> GetItkPoint(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);

    mitk::Vector3D              m_Spacing;
    mitk::Point3D               m_Origin;
    itk::Matrix<double, 3, 3>   m_DirectionMatrix;
    ImageRegion<3>              m_ImageRegion;

    ItkTensorImageType::Pointer         m_TensorImage;
    typename OutputImageType::Pointer   m_DiffusionImage;

    GradientListType    m_GradientList;
    FiberBundleType     m_FiberBundle;
};
}

#include "itkTractsToDWIImageFilter.cpp"

#endif
