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

#include "mitkGLUT.h"

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

static void WriteTextXY(float x, float y, const std::string & text)
{
  glRasterPos2f ( x, y);
  for (unsigned int i = 0; i < text.size(); i++)
    glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, text[i]);
}

static bool makePerpendicularVector2D(const mitk::Vector2D& in, mitk::Vector2D& out)
{
  if((fabs(in[0])>0) && ( (fabs(in[0])>fabs(in[1])) || (in[1] == 0) ) )
  {
    out[0]=-in[1]/in[0];
    out[1]=1;
    out.Normalize();
    return true;
  }
  else
  if(fabs(in[1])>0)
  {
    out[0]=1;
    out[1]=-in[0]/in[1];
    out.Normalize();
    return true;
  }
  else
    return false;
}

//##ModelId=3F0189F00373
void mitk::PointSetMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  const int text2dDistance = 5;

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

    bool showDistances = false;
    this->GetDataTreeNode()->GetBoolProperty("show distances", showDistances);

    bool showAngles = false;
    this->GetDataTreeNode()->GetBoolProperty("show angles", showAngles);

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

    Point3D p;                      // currently visited point 
    Point3D lastP;                  // last visited point 
    Vector3D vec;                   // p - lastP
    Vector3D lastVec;               // lastP - point before lastP
    vec.Fill(0);

    mitk::Point3D projected_p;      // p projected on viewplane

    Point2D pt2d;       // projected_p in display coordinates
    Point2D lastPt2d;   // last projected_p in display coordinates
    Point2D preLastPt2d;// projected_p in display coordinates before lastPt2d

    while(it!=end)
    {
      lastP = p;        // valid only for counter > 0
      lastVec = vec;    // valid only for counter > 1

      preLastPt2d = lastPt2d; // valid only for counter > 1
      lastPt2d = pt2d;  // valid only for counter > 0

      float vtkp[3];

      itk2vtk(it->Value(), vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);

      vec = p-lastP;    // valid only for counter > 0

      displayGeometry->Project(p, projected_p);
      Vector3D diff=p-projected_p;
      ScalarType scalardiff = diff.GetSquaredNorm();
      if((scalardiff<4.0) || (m_Polygon))
      {
        Point2D tmp;
        displayGeometry->Map(projected_p, pt2d);
        displayGeometry->WorldToDisplay(pt2d, pt2d);

        Vector2D horz,vert;
        horz[0]=8.0-scalardiff*2; horz[1]=0;
        vert[0]=0;                vert[1]=8.0-scalardiff*2;

        // now paint text if available
        if (dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer()) != NULL)
        {
          const char * pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer())->GetValue();
          char buffer[20];
          std::string l = pointLabel;
          if (input->GetSize()>1)
          {
            sprintf(buffer,"%d",j+1);
            l.append(buffer);
          }
          WriteTextXY(pt2d[0] + text2dDistance, pt2d[1] + text2dDistance, l);
        }

        if((showPoints) && (scalardiff<4.0))
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
          if(showDistances)
          {
            std::stringstream buffer;
            buffer << vec.GetNorm();

            Vector2D vec2d = pt2d-lastPt2d;
            makePerpendicularVector2D(vec2d, vec2d);

            Vector2D pos2d = (lastPt2d.GetVectorFromOrigin()+pt2d)*0.5+vec2d*text2dDistance;

            WriteTextXY(pos2d[0], pos2d[1], buffer.str());
          }
          if(showAngles && counter > 1 )
          {
            std::stringstream buffer;
            buffer << angle(vec.Get_vnl_vector(), -lastVec.Get_vnl_vector())*180/vnl_math::pi << "°";

            Vector2D vec2d = pt2d-lastPt2d;
            vec2d.Normalize();
            Vector2D lastVec2d = lastPt2d-preLastPt2d;
            lastVec2d.Normalize();
            vec2d=vec2d-lastVec2d;
            vec2d.Normalize();

            Vector2D pos2d = lastPt2d.GetVectorFromOrigin()+vec2d*text2dDistance*text2dDistance;

            WriteTextXY(pos2d[0], pos2d[1], buffer.str());
          }
        }
        counter++;
      }
      ++it;
      ++selIt;
      j++;
    }
  }
}
