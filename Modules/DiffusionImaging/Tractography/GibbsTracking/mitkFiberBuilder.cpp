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

#include "mitkFiberBuilder.h"

using namespace mitk;

FiberBuilder::FiberBuilder(ParticleGrid* grid, ItkFloatImageType* image)
{
    m_Grid = grid;
    m_Image = image;
    m_FiberLength = 0;
}

FiberBuilder::~FiberBuilder()
{

}

vtkSmartPointer<vtkPolyData> FiberBuilder::iterate(int minFiberLength)
{
    m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    int cur_label = 1;
    int numFibers = 0;
    m_FiberLength = 0;
    for (int k = 0; k < m_Grid->m_NumParticles;k++)
    {
        Particle *dp =  m_Grid->GetParticle(k);
        if (dp->label == 0)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            dp->label = cur_label;
            LabelPredecessors(dp, -1, container);
            LabelSuccessors(dp, 1, container);
            cur_label++;
            if(m_FiberLength >= minFiberLength)
            {
                m_VtkCellArray->InsertNextCell(container);
                numFibers++;
            }
            m_FiberLength = 0;
        }
    }
    for (int k = 0; k < m_Grid->m_NumParticles;k++)
    {
        Particle *dp =  m_Grid->GetParticle(k);
        dp->label = 0;
    }

    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    fiberPolyData->SetPoints(m_VtkPoints);
    fiberPolyData->SetLines(m_VtkCellArray);
    return fiberPolyData;
}

void FiberBuilder::LabelPredecessors(Particle* p, int ep, vtkPolyLine* container)
{
    Particle* p2 = NULL;
    if (ep==1)
        p2 = m_Grid->GetParticle(p->pID);
    else
        p2 = m_Grid->GetParticle(p->mID);

    if (p2!=NULL && p2->label==0)
    {
        p2->label = 1;    // assign particle to current fiber

        if (p2->pID==p->ID)
            LabelPredecessors(p2, -1, container);
        else if (p2->mID==p->ID)
            LabelPredecessors(p2, 1, container);
        else
            std::cout << "FiberBuilder: connection inconsistent (LabelPredecessors)" << std::endl;
    }

    AddPoint(p, container);
}


void FiberBuilder::LabelSuccessors(Particle* p, int ep, vtkPolyLine* container)
{
    AddPoint(p, container);

    Particle* p2 = NULL;
    if (ep==1)
        p2 = m_Grid->GetParticle(p->pID);
    else
        p2 = m_Grid->GetParticle(p->mID);

    if (p2!=NULL && p2->label==0)
    {
        p2->label = 1;    // assign particle to current fiber

        if (p2->pID==p->ID)
            LabelSuccessors(p2, -1, container);
        else if (p2->mID==p->ID)
            LabelSuccessors(p2, 1, container);
        else
            std::cout << "FiberBuilder: connection inconsistent (LabelPredecessors)" << std::endl;
    }
}

void FiberBuilder::AddPoint(Particle *dp, vtkSmartPointer<vtkPolyLine> container)
{
    if (dp->label!=1)
        return;

    dp->label = 2;

    itk::ContinuousIndex<float, 3> index;
    index[0] = dp->pos[0]/m_Image->GetSpacing()[0]-0.5;
    index[1] = dp->pos[1]/m_Image->GetSpacing()[1]-0.5;
    index[2] = dp->pos[2]/m_Image->GetSpacing()[2]-0.5;
    itk::Point<float> point;
    m_Image->TransformContinuousIndexToPhysicalPoint( index, point );
    vtkIdType id = m_VtkPoints->InsertNextPoint(point.GetDataPointer());
    container->GetPointIds()->InsertNextId(id);

    if(container->GetNumberOfPoints()>1)
        m_FiberLength += m_LastPoint.EuclideanDistanceTo(point);

    m_LastPoint = point;
}
