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


#include "mitkContourModelGLMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkContourModel.h"
#include "mitkContourModelSubDivisionFilter.h"
#include <vtkLinearTransform.h>

#include "mitkGL.h"

mitk::ContourModelGLMapper2D::ContourModelGLMapper2D()
{
}

mitk::ContourModelGLMapper2D::~ContourModelGLMapper2D()
{
}


void mitk::ContourModelGLMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) return;

  bool updateNeccesary=true;

  int timestep = renderer->GetTimeStep();

  mitk::ContourModel::Pointer input =  const_cast<mitk::ContourModel*>(this->GetInput());
  mitk::ContourModel::Pointer renderingContour = input;

  bool subdivision = false;
  this->GetDataNode()->GetBoolProperty( "subdivision curve", subdivision, renderer );
  if (subdivision)
  {

    mitk::ContourModel::Pointer subdivContour = mitk::ContourModel::New();

    mitk::ContourModelSubDivisionFilter::Pointer subdivFilter = mitk::ContourModelSubDivisionFilter::New();

    subdivFilter->SetInput(input);
    subdivFilter->Update();

    subdivContour = subdivFilter->GetOutput();

    if(subdivContour->GetNumberOfVertices() == 0 )
    {
      subdivContour = input;
    }

    renderingContour = subdivContour;
  }

  renderingContour->UpdateOutputInformation();


  if( renderingContour->GetMTime() < this->m_LastUpdateTime )
    updateNeccesary = false;

  if(renderingContour->GetNumberOfVertices(timestep) < 1)
    updateNeccesary = false;

  if (updateNeccesary)
  {
    // ok, das ist aus GenerateData kopiert
    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
    assert(displayGeometry.IsNotNull());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty("contour.color", renderer));
    if(colorprop)
    {
      //set the color of the contour
      double red = colorprop->GetColor().GetRed();
      double green = colorprop->GetColor().GetGreen();
      double blue = colorprop->GetColor().GetBlue();
      glColor4f(red,green,blue,0.5);
    }

    mitk::ColorProperty::Pointer selectedcolor = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty("points.color", renderer));
    if(!selectedcolor)
    {
      selectedcolor = mitk::ColorProperty::New(1.0,0.0,0.1);
    }


    vtkLinearTransform* transform = GetDataNode()->GetVtkTransform();

    //    ContourModel::OutputType point;
    mitk::Point3D point;

    mitk::Point3D p, projected_p;
    float vtkp[3];
    float lineWidth = 3.0;

    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("contour.width")) != NULL)
      lineWidth = dynamic_cast<mitk::FloatProperty*>(this->GetDataNode()->GetProperty("contour.width"))->GetValue();
    glLineWidth(lineWidth);


    bool drawit=false;

    mitk::ContourModel::VertexIterator pointsIt = renderingContour->IteratorBegin(timestep);

    Point2D pt2d;       // projected_p in display coordinates
    Point2D lastPt2d;

    while ( pointsIt != renderingContour->IteratorEnd(timestep) )
    {
      lastPt2d = pt2d;

      point = (*pointsIt)->Coordinates;

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);

      displayGeometry->Project(p, projected_p);

      displayGeometry->Map(projected_p, pt2d);
      displayGeometry->WorldToDisplay(pt2d, pt2d);

      Vector3D diff=p-projected_p;
      ScalarType scalardiff = diff.GetSquaredNorm();

      //draw lines
      bool projectmode=false;
      GetDataNode()->GetVisibility(projectmode, renderer, "project");

      if(projectmode)
        drawit=true;
      else
      {
        if(diff.GetSquaredNorm()<0.5)
          drawit=true;
      }
      if(drawit)
      {
        //lastPt2d is not valid in first step
        if( !(pointsIt == renderingContour->IteratorBegin(timestep)) )
        {
          glBegin (GL_LINES);
          glVertex2f(pt2d[0], pt2d[1]);
          glVertex2f(lastPt2d[0], lastPt2d[1]);
          glEnd();
        }


        //draw active points
        if ((*pointsIt)->IsControlPoint)
        {
          float pointsize = 4;
          Point2D  tmp;

          Vector2D horz,vert;
          horz[0]=pointsize-scalardiff*2; horz[1]=0;
          vert[0]=0;                vert[1]=pointsize-scalardiff*2;
          horz[0]=pointsize;
          vert[1]=pointsize;
          glColor3f(selectedcolor->GetColor().GetRed(), selectedcolor->GetColor().GetBlue(), selectedcolor->GetColor().GetGreen());
          glLineWidth(1);
          //a rectangle around the point with the selected color
          glBegin (GL_LINE_LOOP);
          tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
          tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
          tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
          tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
          glEnd ();
          glLineWidth(1);
          //the actual point in the specified color to see the usual color of the point
          glColor3f(colorprop->GetColor().GetRed(),colorprop->GetColor().GetGreen(),colorprop->GetColor().GetBlue());
          glPointSize(1);
          glBegin (GL_POINTS);
          tmp=pt2d;             glVertex2fv(&tmp[0]);
          glEnd ();
        }
      }

      pointsIt++;
    }//end while iterate over controlpoints

    //close contour if necessary
    if(renderingContour->IsClosed(timestep) && drawit)
    {
      lastPt2d = pt2d;
      point = renderingContour->GetVertexAt(0,timestep)->Coordinates;
      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);
      displayGeometry->Project(p, projected_p);
      displayGeometry->Map(projected_p, pt2d);
      displayGeometry->WorldToDisplay(pt2d, pt2d);

      glBegin (GL_LINES);
      glVertex2f(lastPt2d[0], lastPt2d[1]);
      glVertex2f( pt2d[0], pt2d[1] );
      glEnd();
    }

    //draw selected vertex if exists
    if(renderingContour->GetSelectedVertex())
    {
      //transform selected vertex
      point = renderingContour->GetSelectedVertex()->Coordinates;

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);

      displayGeometry->Project(p, projected_p);

      displayGeometry->Map(projected_p, pt2d);
      displayGeometry->WorldToDisplay(pt2d, pt2d);

      Vector3D diff=p-projected_p;
      ScalarType scalardiff = diff.GetSquaredNorm();
      //----------------------------------

      //draw point if close to plane
      if(scalardiff<0.5)
      {

        float pointsize = 3.2;
        Point2D  tmp;
        glColor3f(0.0, 1.0, 0.0);
        glLineWidth(1);
        //a diamond around the point
        glBegin (GL_LINE_LOOP);
        //begin from upper left corner and paint clockwise
        tmp[0]=pt2d[0]-pointsize;    tmp[1]=pt2d[1]+pointsize;    glVertex2fv(&tmp[0]);
        tmp[0]=pt2d[0]+pointsize;    tmp[1]=pt2d[1]+pointsize;    glVertex2fv(&tmp[0]);
        tmp[0]=pt2d[0]+pointsize;    tmp[1]=pt2d[1]-pointsize;    glVertex2fv(&tmp[0]);
        tmp[0]=pt2d[0]-pointsize;    tmp[1]=pt2d[1]-pointsize;    glVertex2fv(&tmp[0]);
        glEnd ();
      }
      //------------------------------------
    }
  }
}

const mitk::ContourModel* mitk::ContourModelGLMapper2D::GetInput(void)
{
  return static_cast<const mitk::ContourModel * > ( GetData() );
}

void mitk::ContourModelGLMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "contour.color", ColorProperty::New(0.9, 1.0, 0.1), renderer, overwrite );
  node->AddProperty( "points.color", ColorProperty::New(1.0, 0.0, 0.1), renderer, overwrite );
  node->AddProperty( "contour.width", mitk::FloatProperty::New( 1.0 ), renderer, overwrite );

  node->AddProperty( "subdivision curve", mitk::BoolProperty::New( false ), renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
