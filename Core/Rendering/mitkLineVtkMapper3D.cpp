/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkLineVtkMapper3D.h"

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProp3DCollection.h>
#include <mitkProperties.h>
#include "mitkStringProperty.h"
#include <vtkProperty.h>

mitk::LineVtkMapper3D::LineVtkMapper3D()
: PointSetVtkMapper3D()//constructor of superclass
{
}

mitk::LineVtkMapper3D::~LineVtkMapper3D()
{
}

void mitk::LineVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{//from PointListVTKMapper3D and a little re-layouted!
  if(IsVisible(renderer)==false)
  {
      m_Actor->VisibilityOff();
      return;
  }

  m_Actor->VisibilityOn();

	m_vtkPointList->Delete();
	m_vtkTextList->Delete();
	m_contour->Delete();
	m_tubefilter->Delete();

	m_vtkPointList = vtkAppendPolyData::New();
	m_vtkTextList = vtkAppendPolyData::New();
	m_contour = vtkPolyData::New();
	m_tubefilter = vtkTubeFilter::New();


  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
  mitk::PointSet::PointSetType::Pointer pointList;

	pointList = input->GetPointList();

  mitk::PointSet::PointsContainer::Iterator i;

	int j;
  bool makeContour;
	if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("show contour").GetPointer()) == NULL)
		makeContour = false;
	else
		makeContour = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("show contour").GetPointer())->GetValue();

  vtkPoints *points = vtkPoints::New();
  vtkCellArray *polys = vtkCellArray::New();

  for (j=0, i=pointList->GetPoints()->Begin(); i!=pointList->GetPoints()->End() ; i++,j++)
  {
    int cell[2] = {j-1,j};
    points->InsertPoint(j,i.Value()[0],i.Value()[1],i.Value()[2]);
    if (j>0)
  	  polys->InsertNextCell(2,cell);
  }

  bool close;
  if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("close contour").GetPointer()) == NULL)
    close = false;
  else
    close = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("close contour").GetPointer())->GetValue();

  if (close) 
  {
    int cell[2] = {j-1,0};
    polys->InsertNextCell(2,cell);
  }

  m_contour->SetPoints(points);
  points->Delete();
  m_contour->SetLines(polys);
  polys->Delete();
  m_contour->Update();

  m_tubefilter->SetInput(m_contour);
  m_tubefilter->SetRadius(1);
  m_tubefilter->Update();;

	m_vtkPointList->AddInput(m_tubefilter->GetOutput());
	

  // check for color prop and use it for rendering if it exists
  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  GetColor(rgba, renderer);

  for (j=0, i=pointList->GetPoints()->Begin(); i!=pointList->GetPoints()->End() ; i++,j++)
  {
	  vtkSphereSource *sphere = vtkSphereSource::New();

		sphere->SetRadius(2);
    sphere->SetCenter(i.Value()[0],i.Value()[1],i.Value()[2]);

		m_vtkPointList->AddInput(sphere->GetOutput());

		if (dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer()) == NULL)
		{
    }
		else 
    {
	  	const char * pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer())->GetValue();
			char buffer[20];
 	    std::string l = pointLabel;
			if (input->GetSize()>1)
			{
				sprintf(buffer,"%d",j+1);
				l.append(buffer);
			}

			// Define the text for the label
			vtkVectorText *label = vtkVectorText::New();
			label->SetText(l.c_str());

			// Set up a transform to move the label to a new position.
			vtkTransform *aLabelTransform =vtkTransform::New();
			aLabelTransform->Identity();
			aLabelTransform->Translate(i.Value()[0]+2,i.Value()[1]+2,i.Value()[2]);
			aLabelTransform->Scale(5.7,5.7,5.7);

			// Move the label to a new position.
			vtkTransformPolyDataFilter *labelTransform = vtkTransformPolyDataFilter::New();
			labelTransform->SetTransform(aLabelTransform);
			labelTransform->SetInput(label->GetOutput());

			m_vtkPointList->AddInput(labelTransform->GetOutput());
		}


	}


  m_VtkPolyDataMapper->SetInput(m_vtkPointList->GetOutput());
  m_Actor->GetProperty()->SetColor(rgba);
}

