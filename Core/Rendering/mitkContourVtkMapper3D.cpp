/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkContourVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkOpenGLRenderer.h"


#include <vtkActor.h>
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkFollower.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkTubeFilter.h>
#include <vtkPolygon.h>


#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <stdlib.h>


void mitk::ContourVtkMapper3D::GenerateData()
{
}


mitk::ContourVtkMapper3D::ContourVtkMapper3D()
{
    m_VtkPolyDataMapper = vtkPolyDataMapper::New();
    m_Actor = vtkActor::New();
    m_Actor->SetMapper(m_VtkPolyDataMapper);

    m_Contour = vtkPolyData::New();
 	  m_TubeFilter = vtkTubeFilter::New();


}

mitk::ContourVtkMapper3D::~ContourVtkMapper3D()
{
    m_VtkPolyDataMapper->Delete();
    m_Actor->Delete();
}

void mitk::ContourVtkMapper3D::Update(mitk::BaseRenderer* renderer)
{
    if(IsVisible(renderer)==false)
    {
        m_Actor->VisibilityOff();
        return;
    }
    m_Actor->VisibilityOn();

		m_Contour = vtkPolyData::New();


    mitk::Contour::Pointer input  = const_cast<mitk::Contour*>(this->GetInput());
		bool makeContour = true;

    if (makeContour)
		{
		  vtkPolyData *m_Contour = vtkPolyData::New();
		  vtkPoints *points = vtkPoints::New();
		  vtkCellArray *lines = vtkCellArray::New();
     
      Contour::InputType idx = input->GetContourPath()->StartOfInput();
      Contour::OutputType point;

      Contour::InputType end = input->GetContourPath()->EndOfInput();
      if (end > 50000) end = 0;
      vtkIdType ptIndex = 0;

      while ( idx != end )
      {
        point = input->GetContourPath()->Evaluate(idx);
				int cell[2] = {ptIndex-1,ptIndex};
        points->InsertPoint(ptIndex, point[0],point[1],point[2]);
				if (ptIndex>0)
        {
					lines->InsertNextCell(2,cell);
        }
        ptIndex++;
        idx+=1;
			}

      int cell[2] = {ptIndex-1,0};
			lines->InsertNextCell(2,cell);
      

			m_Contour->SetPoints(points);
		  points->Delete();
  		m_Contour->SetLines(lines);
			m_Contour->Update();

		  m_TubeFilter->SetInput(m_Contour);
		  m_TubeFilter->SetRadius(1);
      m_TubeFilter->Update();
      m_VtkPolyDataMapper->SetInput(m_TubeFilter->GetOutput());
      float rgba[4]={0.0f,1.0f,0.0f,0.6f};
      m_Actor->GetProperty()->SetColor(rgba);
      m_Actor->SetMapper(m_VtkPolyDataMapper);
    }

  
    StandardUpdate();
}

void mitk::ContourVtkMapper3D::Update()
{
}

vtkProp* mitk::ContourVtkMapper3D::GetProp()
{
  return m_Actor;
}

void mitk::ContourVtkMapper3D::GenerateOutputInformation()
{
    mitk::Image::Pointer output = this->GetOutput();
    mitk::PixelType pt( typeid( int ) );
    unsigned int dim[] = {256, 256};
    output->Initialize( mitk::PixelType( typeid( short int ) ), 2, dim, 10 );
    SlicedData::RegionType::SizeType size = {0,0,0,0,0};
    SlicedData::RegionType region(size);
    output->SetRequestedRegion(&region);
}


const mitk::Contour* mitk::ContourVtkMapper3D::GetInput()
{
    if (this->GetNumberOfInputs() < 1)
    {
        return 0;
    }
    return static_cast<const mitk::Contour* > ( GetData() );
}
