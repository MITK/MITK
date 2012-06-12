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
    particles = m_Grid->m_Particles;
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
        Particle *dp =  &(particles[k]);
        if (dp->label == 0)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            dp->label = cur_label;
            dp->numerator = 0;
            labelPredecessors(dp, container);
            labelSuccessors(dp, container);
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
        Particle *dp =  &(particles[k]);
        dp->inserted = false;
        dp->label = 0;
    }

    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    fiberPolyData->SetPoints(m_VtkPoints);
    fiberPolyData->SetLines(m_VtkCellArray);
    return fiberPolyData;
}

void FiberBuilder::AddPoint(Particle *dp, vtkSmartPointer<vtkPolyLine> container)
{
    if (dp->inserted)
        return;

    dp->inserted = true;

    itk::ContinuousIndex<float, 3> index;
    index[0] = dp->R[0]/m_Image->GetSpacing()[0];
    index[1] = dp->R[1]/m_Image->GetSpacing()[1];
    index[2] = dp->R[2]/m_Image->GetSpacing()[2];
    itk::Point<float> point;
    m_Image->TransformContinuousIndexToPhysicalPoint( index, point );
    vtkIdType id = m_VtkPoints->InsertNextPoint(point.GetDataPointer());
    container->GetPointIds()->InsertNextId(id);

    if(container->GetNumberOfPoints()>1)
        m_FiberLength += m_LastPoint.EuclideanDistanceTo(point);

    m_LastPoint = point;
}

void FiberBuilder::labelPredecessors(Particle *dp, vtkSmartPointer<vtkPolyLine> container)
{
    if (dp->mID != -1 && dp->mID!=dp->ID)
    {
        if (dp->ID!=m_Grid->GetParticle(dp->mID)->pID)
        {
            if (dp->ID==m_Grid->GetParticle(dp->mID)->mID)
            {
                int tmp = m_Grid->GetParticle(dp->mID)->pID;
                m_Grid->GetParticle(dp->mID)->pID = m_Grid->GetParticle(dp->mID)->mID;
                m_Grid->GetParticle(dp->mID)->mID = tmp;
            }
        }
        if (m_Grid->GetParticle(dp->mID)->label == 0)
        {
            m_Grid->GetParticle(dp->mID)->label = dp->label;
            m_Grid->GetParticle(dp->mID)->numerator = dp->numerator-1;
            labelPredecessors(m_Grid->GetParticle(dp->mID), container);
        }
    }

    AddPoint(dp, container);
}

void FiberBuilder::labelSuccessors(Particle *dp, vtkSmartPointer<vtkPolyLine> container)
{
    AddPoint(dp, container);

    if (dp->pID != -1 && dp->pID!=dp->ID)
    {
        if (dp->ID!=m_Grid->GetParticle(dp->pID)->mID)
        {
            if (dp->ID==m_Grid->GetParticle(dp->pID)->pID)
            {
                int tmp = m_Grid->GetParticle(dp->pID)->pID;
                m_Grid->GetParticle(dp->pID)->pID = m_Grid->GetParticle(dp->pID)->mID;
                m_Grid->GetParticle(dp->pID)->mID = tmp;
            }
        }
        if (m_Grid->GetParticle(dp->pID)->label == 0)
        {
            m_Grid->GetParticle(dp->pID)->label = dp->label;
            m_Grid->GetParticle(dp->pID)->numerator = dp->numerator+1;
            labelSuccessors(m_Grid->GetParticle(dp->pID), container);
        }
    }
}
