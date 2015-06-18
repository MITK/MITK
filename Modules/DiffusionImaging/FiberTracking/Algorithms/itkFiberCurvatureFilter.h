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
#ifndef itkFiberCurvatureFilter_h
#define itkFiberCurvatureFilter_h

// MITK
#include <mitkPlanarEllipse.h>
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>

// ITK
#include <itkProcessObject.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

using namespace std;

namespace itk{

/**
* \brief Generates artificial fibers distributed in and interpolated between the input planar figures.   */

class FiberCurvatureFilter : public ProcessObject
{
public:

    typedef FiberCurvatureFilter Self;
    typedef ProcessObject                                       Superclass;
    typedef SmartPointer< Self >                                Pointer;
    typedef SmartPointer< const Self >                          ConstPointer;

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    itkTypeMacro( FiberCurvatureFilter, ProcessObject )

    virtual void Update() override{
        this->GenerateData();
    }

    itkSetMacro( Distance, double )
    itkSetMacro( AngularDeviation, double )
    itkSetMacro( RemoveFibers, bool )
    itkSetMacro( InputFiberBundle, FiberBundle::Pointer )
    itkGetMacro( OutputFiberBundle, FiberBundle::Pointer )

protected:

    void GenerateData() override;

    FiberCurvatureFilter();
    virtual ~FiberCurvatureFilter();

    FiberBundle::Pointer m_InputFiberBundle;
    FiberBundle::Pointer m_OutputFiberBundle;
    double  m_AngularDeviation;
    double  m_Distance;
    bool    m_RemoveFibers;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFiberCurvatureFilter.cpp"
#endif

#endif
