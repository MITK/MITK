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
  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  mitk::DataNode* dataNode = this->GetDataNode();

  bool visible = true;
  dataNode->GetVisibility(visible, renderer, "visible");

  if ( !visible ) return;

  bool updateNeccesary=true;

  int timestep = renderer->GetTimeStep();

  mitk::ContourModel::Pointer input =  const_cast<mitk::ContourModel*>(this->GetInput());
  mitk::ContourModel::Pointer renderingContour = input;

  bool subdivision = false;

  dataNode->GetBoolProperty( "subdivision curve", subdivision, renderer );
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


  if( renderingContour->GetMTime() < ls->GetLastGenerateDataTime() )
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

    bool isEditing = false;
    dataNode->GetBoolProperty("contour.editing", isEditing);

    mitk::ColorProperty::Pointer colorprop;

    if (isEditing)
        colorprop = dynamic_cast<mitk::ColorProperty*>(dataNode->GetProperty("contour.editing.color", renderer));
    else
        colorprop = dynamic_cast<mitk::ColorProperty*>(dataNode->GetProperty("contour.color", renderer));

    if(colorprop)
    {
      //set the color of the contour
      double red = colorprop->GetColor().GetRed();
      double green = colorprop->GetColor().GetGreen();
      double blue = colorprop->GetColor().GetBlue();
      glColor4f(red,green,blue,0.5);
    }

    mitk::ColorProperty::Pointer selectedcolor = dynamic_cast<mitk::ColorProperty*>(dataNode->GetProperty("contour.points.color", renderer));
    if(!selectedcolor)
    {
      selectedcolor = mitk::ColorProperty::New(1.0,0.0,0.1);
    }


    vtkLinearTransform* transform = dataNode->GetVtkTransform();

    //    ContourModel::OutputType point;
    mitk::Point3D point;

    mitk::Point3D p, projected_p;
    float vtkp[3];
    float lineWidth = 3.0;

    bool isHovering = false;
    dataNode->GetBoolProperty("contour.hovering", isHovering);

    if (isHovering)
        dataNode->GetFloatProperty("contour.hovering.width", lineWidth);
    else
        dataNode->GetFloatProperty("contour.width", lineWidth);

    bool drawit=false;

    mitk::ContourModel::VertexIterator pointsIt = renderingContour->IteratorBegin(timestep);

    Point2D pt2d;       // projected_p in display coordinates
    Point2D lastPt2d;

    int index = 0;

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
      ScalarType scalardiff = diff.GetNorm();

      //draw lines
      bool projectmode=false;
      dataNode->GetVisibility(projectmode, renderer, "contour.project-onto-plane");

      if(projectmode)
      {
        drawit=true;
      }
      else if(scalardiff<0.25)
      {
        drawit=true;
      }

      if(drawit)
      {
        //lastPt2d is not valid in first step
        if( !(pointsIt == renderingContour->IteratorBegin(timestep)) )
        {
          glLineWidth(lineWidth);
          glBegin (GL_LINES);
          glVertex2f(pt2d[0], pt2d[1]);
          glVertex2f(lastPt2d[0], lastPt2d[1]);
          glEnd();
          glLineWidth(1);
        }

        bool showControlPoints = false;
        dataNode->GetBoolProperty("contour.controlpoints.show", showControlPoints);

        if (showControlPoints)
        {
            //draw active points
            if ((*pointsIt)->IsControlPoint)
            {
              float pointsize = 4;
              Point2D  tmp;

              Vector2D horz,vert;
              horz[1]=0;
              vert[0]=0;
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
              glEnd();
              glLineWidth(1);
              //the actual point in the specified color to see the usual color of the point
              glColor3f(colorprop->GetColor().GetRed(),colorprop->GetColor().GetGreen(),colorprop->GetColor().GetBlue());
              glPointSize(1);
              glBegin (GL_POINTS);
              tmp=pt2d;             glVertex2fv(&tmp[0]);
              glEnd ();
            }
        }

        bool showPoints = false;
        dataNode->GetBoolProperty("contour.points.show", showPoints);

        if (showPoints)
        {
          float pointsize = 3;
          Point2D  tmp;

          Vector2D horz,vert;
          horz[1]=0;
          vert[0]=0;
          horz[0]=pointsize;
          vert[1]=pointsize;
          glColor3f(0.0, 0.0, 0.0);
          glLineWidth(1);
          //a rectangle around the point with the selected color
          glBegin (GL_LINE_LOOP);
          tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
          tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
          tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
          tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
          glEnd();
          glLineWidth(1);
          //the actual point in the specified color to see the usual color of the point
          glColor3f(colorprop->GetColor().GetRed(),colorprop->GetColor().GetGreen(),colorprop->GetColor().GetBlue());
          glPointSize(1);
          glBegin (GL_POINTS);
          tmp=pt2d;             glVertex2fv(&tmp[0]);
          glEnd ();
        }

/*
        std::string l;
        std::stringstream ss;
        ss << index;
        l.append(ss.str());

        mitk::VtkPropRenderer* OpenGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
        float rgb[3];
        rgb[0] = 0.0; rgb[1] = 0.0; rgb[2] = 0.0;
        OpenGLrenderer->WriteSimpleText(l, pt2d[0] + 2, pt2d[1] + 2,rgb[0], rgb[1],rgb[2]);

        index++;
*/
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

      glLineWidth(lineWidth);
      glBegin (GL_LINES);
      glVertex2f(lastPt2d[0], lastPt2d[1]);
      glVertex2f( pt2d[0], pt2d[1] );
      glEnd();
      glLineWidth(1);
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
      ScalarType scalardiff = diff.GetNorm();
      //----------------------------------

      //draw point if close to plane
      if(scalardiff<0.25)
      {

        float pointsize = 5;
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
  return static_cast<const mitk::ContourModel * > ( GetDataNode()->GetData() );
}

void mitk::ContourModelGLMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "contour.color", ColorProperty::New(0.9, 1.0, 0.1), renderer, overwrite );
  node->AddProperty( "contour.editing", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.editing.color", ColorProperty::New(0.1, 0.9, 0.1), renderer, overwrite );
  node->AddProperty( "contour.points.color", ColorProperty::New(1.0, 0.0, 0.1), renderer, overwrite );
  node->AddProperty( "contour.points.show", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.controlpoints.show", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.width", mitk::FloatProperty::New( 1.0 ), renderer, overwrite );
  node->AddProperty( "contour.hovering.width", mitk::FloatProperty::New( 3.0 ), renderer, overwrite );
  node->AddProperty( "contour.hovering", mitk::BoolProperty::New( false ), renderer, overwrite );

  node->AddProperty( "subdivision curve", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.project-onto-plane", mitk::BoolProperty::New( false ), renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
