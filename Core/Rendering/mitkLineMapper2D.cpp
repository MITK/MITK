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


#include "mitkLineMapper2D.h"
#include "mitkPointSet.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include <vtkLinearTransform.h>
#include "mitkPointSet.h"
#include "mitkGL.h"
	
mitk::LineMapper2D::LineMapper2D()
: mitk::PointSetMapper2D()
{
 
}

mitk::LineMapper2D::~LineMapper2D()
{
}

const mitk::PointSet *mitk::LineMapper2D::GetInput(void)
{
  return static_cast<const mitk::PointSet * > ( GetData() );
}

void mitk::LineMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) 
    return;

  bool updateNeccesary = true;//!!!! @TODO !??!!

  if (updateNeccesary) 
  {
    mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
    assert(displayGeometry.IsNotNull());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    vtkLinearTransform* transform = GetDataTreeNode()->GetVtkTransform();

    //List of the Points
    PointSet::DataType::PointsContainerConstIterator it, end;
    it = input->GetPointSet()->GetPoints()->Begin();
    end = input->GetPointSet()->GetPoints()->End();//the last before end, because lines from n to n+1
		if (end!=it)  // otherwise PointSet is empty
				end--;

    //bool list for the selection of the points
    PointSet::DataType::PointDataContainerIterator selIt, selItNext;
    selIt=input->GetPointSet()->GetPointData()->Begin();
    
    int j=0;
    while(it!=end)
    {
      mitk::Point3D p, q, projected_p, projected_q;
      float vtkp[3];
      itk2vtk(it->Value(), vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);
      
      //next point n+1
      it++;
      itk2vtk(it->Value(), vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,q);
      it--;

      displayGeometry->Project(p, projected_p);
      displayGeometry->Project(q, projected_q);

      Vector3D diffp=p-projected_p, diffq=q-projected_q;
      if((diffp.GetSquaredNorm()<4.0) &&
         (diffq.GetSquaredNorm()<4.0))
      {
        Point2D p2d, q2d, tmp;
        
        displayGeometry->Map(projected_p, p2d);
        displayGeometry->WorldToDisplay(p2d, p2d);
        displayGeometry->Map(projected_q, q2d);
        displayGeometry->WorldToDisplay(q2d, q2d);

        //outlined, cause iterators behave strange and multiple points can't be selected anyway by now!
        //selItNext = selIt++;//Iterator to the next Element, cause only when the two points ar selected, then the Line is Selected
        //if (selIt->Value() && selItNext->Value())//selected
        //{
        //  float colorSel[]={1.0,0.0,0.6}; //for selected!

        //  //current color for changing to a diferent color if selected
        //  float currCol[4];
        //  glGetFloatv(GL_CURRENT_COLOR,currCol);

        //  glColor3f(colorSel[0],colorSel[1],colorSel[2]);//red


        //  glBegin (GL_LINES);
        //    glVertex2f(p2d[0], p2d[1]);
        //    glVertex2f(q2d[0], q2d[1]);
        //  glEnd ();

        //  glColor3f(currCol[0],currCol[1],currCol[2]);//the color before changing to select!

        //}
        //else
        //{
					glBegin (GL_LINES);
            glVertex2f(p2d[0], p2d[1]);
            glVertex2f(q2d[0], q2d[1]);
          glEnd ();
        //}
      }
      ++it;
      ++selIt;
 			j++;
    }
    //drawing the points
    PointSetMapper2D::Paint(renderer);
  }
}
