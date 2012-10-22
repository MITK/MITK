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
#ifndef itkFibersFromPointsFilter_h
#define itkFibersFromPointsFilter_h

// MITK
#include <mitkPlanarCircle.h>
#include <mitkFiberBundleX.h>

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

class FibersFromPointsFilter : public ProcessObject
{
public:
    typedef FibersFromPointsFilter Self;
    typedef ProcessObject Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;
    typedef vector< vector< mitk::PlanarCircle::Pointer > > FiducialListType;
    typedef mitk::FiberBundleX::Pointer FiberType;

    itkNewMacro(Self)
    itkTypeMacro( FibersFromPointsFilter, ProcessObject )

    void GenerateData();

    virtual void Update(){
        this->GenerateData();
    }

    void SetFiducials(FiducialListType fiducials){ m_Fiducials = fiducials; }
    itkGetMacro(FiberBundle, FiberType)

protected:

    FibersFromPointsFilter();
    virtual ~FibersFromPointsFilter();

    FiducialListType    m_Fiducials;
    FiberType           m_FiberBundle;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFibersFromPointsFilter.cpp"
#endif

#endif
