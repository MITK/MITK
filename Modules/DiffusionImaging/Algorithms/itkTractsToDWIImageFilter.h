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
#include <mitkParticle.h>
#include <mitkParticleGrid.h>
#include <mitkFiberBundle.h>

// ITK
#include <itkImage.h>
#include <itkProcessObject.h>
#include <itkImageToImageFilter.h>
#include <itkOrientationDistributionFunction.h>

typedef itk::VectorImage< short, 3 > DWIImageType;

namespace itk
{
  class TractsToDWIImageFilter :
      public ImageToImageFilter< DWIImageType, DWIImageType >{

  public:
    typedef TractsToDWIImageFilter Self;
    typedef ImageToImageFilter< DWIImageType, DWIImageType > Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;

    typedef itk::VectorContainer< int, mitk::Particle* > ParticleContainerType;
    typedef mitk::ParticleGrid::Pointer ParticleGridType;

    typedef mitk::FiberBundle::Pointer FiberBundleType;

    typedef vnl_vector_fixed< double, 3 > GradientDirectionType;
    typedef itk::VectorContainer< unsigned int,  GradientDirectionType > GradientDirectionContainerType;

    itkNewMacro(Self);
    itkTypeMacro( TractsToDWIImageFilter, ImageToImageFilter );

    // odf type
    typedef OrientationDistributionFunction<float, QBALL_ODFSIZE> OdfType;

    itkSetMacro(FiberBundle, FiberBundleType);
    itkGetMacro(FiberBundle, FiberBundleType);

    itkSetMacro(ParticleGrid, ParticleGridType);
    itkGetMacro(ParticleGrid, ParticleGridType);

    itkSetMacro(bD, int);
    itkGetMacro(bD, int);

    itkSetMacro(ParticleWidth, float);
    itkGetMacro(ParticleWidth, float);

    itkSetMacro(ParticleLength, float);
    itkGetMacro(ParticleLength, float);

    itkSetMacro(ParticleWeight, float);
    itkGetMacro(ParticleWeight, float);

    itkSetMacro(GradientDirections, GradientDirectionContainerType::Pointer);
    itkGetMacro(GradientDirections, GradientDirectionContainerType::Pointer);

    void GenerateData();

  protected:

    TractsToDWIImageFilter();
    virtual ~TractsToDWIImageFilter();

    GradientDirectionContainerType::Pointer m_GradientDirections;
    FiberBundleType m_FiberBundle;
    ParticleGridType m_ParticleGrid;
    DWIImageType::Pointer m_OutImage;
    int m_Size[3];
    float m_Meanval_sq;

    float *m_BesselApproxCoeff;
    float* ComputeFiberCorrelation();
    float Bessel(float x);
    void GenerateParticleGrid();

    int m_bD;
    float m_ParticleWidth;
    float m_ParticleLength;
    float m_ParticleWeight;
  };
}

#include "itkTractsToDWIImageFilter.cpp"

#endif
