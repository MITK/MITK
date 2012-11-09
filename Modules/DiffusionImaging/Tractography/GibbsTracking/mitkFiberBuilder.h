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
#ifndef _BUILDFIBRES
#define _BUILDFIBRES

// MITK
#include <MitkDiffusionImagingExports.h>
#include <mitkParticleGrid.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCleanPolyData.h>

#include <itkImage.h>

using namespace std;

namespace mitk
{

/**
* \brief Gnerates actual fiber structure (vtkPolyData) from the particle grid content.   */

class MitkDiffusionImaging_EXPORT FiberBuilder
{
public:

    typedef itk::Image<float, 3>  ItkFloatImageType;

    FiberBuilder(ParticleGrid* grid, ItkFloatImageType* image);
    ~FiberBuilder();

    vtkSmartPointer<vtkPolyData> iterate(int minFiberLength);

protected:

    void AddPoint(Particle *dp, vtkSmartPointer<vtkPolyLine> container);

    void LabelPredecessors(Particle* p, int ep, vtkPolyLine* container);
    void LabelSuccessors(Particle* p, int ep, vtkPolyLine* container);

    itk::Point<float>           m_LastPoint;
    float                       m_FiberLength;
    ItkFloatImageType::Pointer  m_Image;
    ParticleGrid*               m_Grid;
    vtkSmartPointer<vtkCellArray> m_VtkCellArray;
    vtkSmartPointer<vtkPoints>    m_VtkPoints;

};

}

#endif
