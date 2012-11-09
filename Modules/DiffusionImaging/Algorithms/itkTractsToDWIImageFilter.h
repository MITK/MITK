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
#include <mitkDiffusionSignalModel.h>
#include <mitkDiffusionNoiseModel.h>

// ITK
#include <itkImage.h>
#include <itkVectorImage.h>
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
    typedef itk::VectorImage< double, 3 >                   DoubleDwiType;
    typedef std::vector< DiffusionSignalModel<double>* >    DiffusionModelList;
    typedef itk::Matrix<double, 3, 3>                       MatrixType;
    typedef mitk::DiffusionNoiseModel<double>               NoiseModelType;

    itkNewMacro(Self)
    itkTypeMacro( TractsToDWIImageFilter, ImageToImageFilter )

    // input
    itkSetMacro(FiberBundle, FiberBundleType)
    itkSetMacro( FiberBaseline, double )
    itkSetMacro( NonFiberBaseline, double )
    itkSetMacro( Spacing, mitk::Vector3D )
    itkSetMacro( Origin, mitk::Point3D )
    itkSetMacro( DirectionMatrix, MatrixType )
    itkSetMacro( ImageRegion, ImageRegion<3> )
    itkSetMacro( TissueMask, ItkUcharImgType::Pointer )
    void SetNoiseModel(NoiseModelType* noiseModel){ m_NoiseModel = noiseModel; }
    void SetFiberModels(DiffusionModelList modelList){ m_FiberModels = modelList; }
    void SetNonFiberModels(DiffusionModelList modelList){ m_NonFiberModels = modelList; }

    void GenerateData();

protected:

    TractsToDWIImageFilter();
    virtual ~TractsToDWIImageFilter();
    itk::Point<float, 3> GetItkPoint(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(double point[3]);
    vnl_vector_fixed<double, 3> GetVnlVector(Vector< float, 3 >& vector);
    void AddNoise(DoubleDwiType::PixelType& pix);

    mitk::Vector3D                      m_Spacing;
    mitk::Point3D                       m_Origin;
    MatrixType                          m_DirectionMatrix;
    ImageRegion<3>                      m_ImageRegion;

    double                              m_FiberBaseline;
    double                              m_NonFiberBaseline;
    ItkUcharImgType::Pointer            m_TissueMask;

    FiberBundleType                     m_FiberBundle;

    DiffusionModelList                  m_FiberModels;
    DiffusionModelList                  m_NonFiberModels;
    NoiseModelType*                     m_NoiseModel;
};
}

#include "itkTractsToDWIImageFilter.cpp"

#endif
