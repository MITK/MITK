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
#include <itkImageSource.h>

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

    itkNewMacro(Self)
    itkTypeMacro( TractsToDWIImageFilter, ImageToImageFilter )

    // input
    itkSetMacro( FiberBundle, FiberBundleType )         ///< input fiber bundle
    itkSetMacro( FiberBaseline, double )                ///< baseline signal of fiber compartments
    itkSetMacro( NonFiberBaseline, double )             ///< baseline signal of non-fiber compartments
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

    mitk::Vector3D                      m_Spacing;              ///< output image spacing
    mitk::Point3D                       m_Origin;               ///< output image origin
    MatrixType                          m_DirectionMatrix;      ///< output image rotation
    ImageRegion<3>                      m_ImageRegion;          ///< output image size
    double                              m_FiberBaseline;        ///< baseline signal of fiber compartments
    double                              m_NonFiberBaseline;     ///< baseline signal of non-fiber compartments
    ItkUcharImgType::Pointer            m_TissueMask;           ///< voxels outside of this binary mask contain only noise (are treated as air)
    FiberBundleType                     m_FiberBundle;          ///< input fiber bundle
    DiffusionModelList                  m_FiberModels;          ///< generate signal of fiber compartments
    DiffusionModelList                  m_NonFiberModels;       ///< generate signal of non-fiber compartments
    NoiseModelType*                     m_NoiseModel;           ///< generates the noise added to the image values
};
}

#include "itkTractsToDWIImageFilter.cpp"

#endif
