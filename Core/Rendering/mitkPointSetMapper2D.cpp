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


#include "mitkPointSetMapper2D.h"
#include "mitkPointSet.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include <vtkLinearTransform.h>
#include "mitkStringProperty.h"
#include "mitkPointSet.h"

#include <GL/glut.h>

//##Documentation
//##@brief color to mark a selected point
const float selectedColor[]={1.0,0.0,0.6}; //for selected!

//##ModelId=3F0189F00378
mitk::PointSetMapper2D::PointSetMapper2D()
: m_Polygon(false) {
}

//##ModelId=3F0189F00382
mitk::PointSetMapper2D::~PointSetMapper2D()
{
}

//##ModelId=3F0189F00366
const mitk::PointSet *mitk::PointSetMapper2D::GetInput(void)
{
    return static_cast<const mitk::PointSet * > ( GetData() );
}

void mitk::PointSetMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{
  GLMapper2D::ApplyProperties( renderer );
  
  const mitk::DataTreeNode* node=GetDataTreeNode();
  if( node == NULL )
    return;

  node->GetBoolProperty( "contour", m_Polygon );
}

//##ModelId=3F0189F00373
void mitk::PointSetMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) return;

  //	@FIXME: Logik fuer update
  bool updateNeccesary=true;

  if (updateNeccesary) {
      // ok, das ist aus GenerateData kopiert

      mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());

      mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();

      assert(displayGeometry.IsNotNull());

      //apply color and opacity read from the PropertyList
      ApplyProperties(renderer);

      bool showPoints = true;
      this->GetDataTreeNode()->GetBoolProperty("show points", showPoints);

      vtkLinearTransform* transform = GetDataTreeNode()->GetVtkTransform();

      //List of the Points
      PointSet::DataType::PointsContainerConstIterator it, end;      
      it=input->GetPointSet()->GetPoints()->Begin();
      end=input->GetPointSet()->GetPoints()->End();

      //iterator on the additional data of each point
      PointSet::DataType::PointDataContainerIterator selIt, selEnd;
      selIt=input->GetPointSet()->GetPointData()->Begin();
      selEnd=input->GetPointSet()->GetPointData()->End();
      int counter = 0;
      
      //for writing text 
      int j=0;

      //for switching back to old color after using selected color
      float unselectedColor[4];
      //get current color to recall colorchange after paint
      glGetFloatv(GL_CURRENT_COLOR,unselectedColor);

      while(it!=end)
      {
          mitk::Point3D p, projected_p;
          float vtkp[3];

          itk2vtk(it->Value(), vtkp);
          transform->TransformPoint(vtkp, vtkp);
          vtk2itk(vtkp,p);

          displayGeometry->Project(p, projected_p);
          Vector3D diff=p-projected_p;
          ScalarType scalardiff = diff.GetSquaredNorm();
          if(scalardiff<4.0)
          {
              Point2D pt2d, tmp, lastPt2d;
              displayGeometry->Map(projected_p, pt2d);
              displayGeometry->WorldToDisplay(pt2d, pt2d);

              Vector2D horz,vert;
              horz[0]=8.0-scalardiff*2; horz[1]=0;
              vert[0]=0;                vert[1]=8.0-scalardiff*2;
                              
							// now paint text if available
              glRasterPos2f ( pt2d[0] + 5, pt2d[1] + 5);
							if (dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer()) == NULL)
							{}
							else {

									const char * pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer())->GetValue();
									char buffer[20];
						 	    std::string l = pointLabel;
									if (input->GetSize()>1)
									{
											sprintf(buffer,"%d",j+1);
											l.append(buffer);
									}
	
    		        for (unsigned int i = 0; i < l.size(); i++)
		    		      glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, l[i]);
		        	}

              if(showPoints)
              {
                //check if the point is to be marked as selected 
                if (selIt->Value().selected)
                {
                  horz[0]=8;
                  vert[1]=8;
                  glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red

                  //a diamond around the point with the selected color
                  glBegin (GL_LINE_LOOP);
                      tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
                      tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
                      tmp=pt2d+horz;			glVertex2fv(&tmp[0]);
                      tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
                  glEnd ();

                  //the actual point in the specified color to see the usual color of the point
                  glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
                  glBegin (GL_POINTS);
                    tmp=pt2d;             glVertex2fv(&tmp[0]);
                  glEnd ();
                }
                else //if not selected
                {
                  glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
								  //drawing crosses
                  glBegin (GL_LINES);
                      tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
                      tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
                      tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
                      tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
                  glEnd ();
                }
              }

              if ( m_Polygon && counter > 0 )
              {
               glBegin (GL_LINES);
                  glVertex2fv(&pt2d[0]);
                  glVertex2fv(&lastPt2d[0]);
               glEnd ();               
              }
            lastPt2d = pt2d;
            counter++;
          }
          ++it;
          ++selIt;
 					j++;
      }
  }
}



