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


#include "mitkPointSetMapper2D.h"
#include "mitkPointSet.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "vtkLinearTransform.h"
#include "mitkStringProperty.h"
#include "mitkPointSet.h"
#include "mitkVtkPropRenderer.h"
#include "mitkGL.h"

//const float selectedColor[]={1.0,0.0,0.6}; //for selected!


mitk::PointSetMapper2D::PointSetMapper2D()
: m_Polygon(false),
  m_ShowPoints(true),
  m_ShowDistances(false),
  m_DistancesDecimalDigits(1),
  m_ShowAngles(false),
  m_ShowDistantLines(true),
  m_LineWidth(1)
{
}


mitk::PointSetMapper2D::~PointSetMapper2D()
{
}


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

  node->GetBoolProperty("show contour",            m_Polygon);
  node->GetBoolProperty("show points",        m_ShowPoints);
  node->GetBoolProperty("show distances",     m_ShowDistances);
  node->GetIntProperty("distance decimal digits",     m_DistancesDecimalDigits);
  node->GetBoolProperty("show angles",        m_ShowAngles);
  node->GetBoolProperty("show distant lines", m_ShowDistantLines);
  node->GetIntProperty("line width",          m_LineWidth);
  node->GetIntProperty("point line width",    m_PointLineWidth);
  node->GetIntProperty("point 2D size",       m_Point2DSize);
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


void mitk::PointSetMapper2D::Paint( mitk::BaseRenderer *renderer )
{

  const mitk::DataTreeNode* node=GetDataTreeNode();
  if( node == NULL )
    return;

  const int text2dDistance = 10;

  if(IsVisible(renderer)==false) return;

  // @FIXME: Logik fuer update
  bool updateNeccesary=true;

  if (updateNeccesary) 
  {
    // ok, das ist aus GenerateData kopiert
    mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());

    // Get the TimeSlicedGeometry of the input object
    const TimeSlicedGeometry* inputTimeGeometry = input->GetTimeSlicedGeometry();
    if (( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeSteps() == 0 ) )
    {
      return;
    }

    //
    // get the world time
    //
    const Geometry2D* worldGeometry = renderer->GetCurrentWorldGeometry2D();
    assert( worldGeometry != NULL );
    ScalarType time = worldGeometry->GetTimeBounds()[ 0 ];

    //
    // convert the world time in time steps of the input object
    //
    int timeStep=0;
    if ( time > ScalarTypeNumericTraits::NonpositiveMin() )
      timeStep = inputTimeGeometry->MSToTimeStep( time );
    if ( inputTimeGeometry->IsValidTime( timeStep ) == false )
    {
      return;
    }


    mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( timeStep );

    if ( itkPointSet.GetPointer() == NULL) 
    {
      return;
    }

    
    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();

    assert(displayGeometry.IsNotNull());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    vtkLinearTransform* transform = GetDataTreeNode()->GetVtkTransform();

    //List of the Points
    PointSet::DataType::PointsContainerConstIterator it, end;      
    it = itkPointSet->GetPoints()->Begin();
    end = itkPointSet->GetPoints()->End();

    //iterator on the additional data of each point
    PointSet::DataType::PointDataContainerIterator selIt, selEnd;
    bool pointDataBroken = (itkPointSet->GetPointData()->Size() != itkPointSet->GetPoints()->Size());
    selIt = itkPointSet->GetPointData()->Begin();
    selEnd = itkPointSet->GetPointData()->End();

    int counter = 0;

    //for writing text 
    int j = 0;

    //for switching back to old color after using selected color
    float recallColor[4];
    glGetFloatv(GL_CURRENT_COLOR,recallColor);
    
    //get the properties for coloring the points
    float unselectedColor[4] = {1.0, 1.0, 0.0, 1.0};//yellow
    //check if there is an unselected property
    if (dynamic_cast<mitk::ColorProperty*>(node->GetPropertyList(renderer)->GetProperty("unselectedcolor")) != NULL)
    {
      mitk::Color tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("unselectedcolor"))->GetValue();
      unselectedColor[0] = tmpColor[0];
      unselectedColor[1] = tmpColor[1];
      unselectedColor[2] = tmpColor[2];
      unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
    }
    else if (dynamic_cast<mitk::ColorProperty*>(node->GetPropertyList(NULL)->GetProperty("unselectedcolor")) != NULL)
    {
      mitk::Color tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("unselectedcolor"))->GetValue();
      unselectedColor[0] = tmpColor[0];
      unselectedColor[1] = tmpColor[1];
      unselectedColor[2] = tmpColor[2];
      unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
    }
    else
    {
      //get the color from the dataTreeNode
      node->GetColor(unselectedColor, NULL);
    }

    //get selected property
    float selectedColor[4] = {1.0, 0.0, 0.6, 1.0};
    if (dynamic_cast<mitk::ColorProperty*>(node->GetPropertyList(renderer)->GetProperty("selectedcolor")) != NULL)
    {
      mitk::Color tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("selectedcolor"))->GetValue();
      selectedColor[0] = tmpColor[0];
      selectedColor[1] = tmpColor[1];
      selectedColor[2] = tmpColor[2];
      selectedColor[3] = 1.0f;
    }
    else if (dynamic_cast<mitk::ColorProperty*>(node->GetPropertyList(NULL)->GetProperty("selectedcolor")) != NULL)
    {
      mitk::Color tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("selectedcolor"))->GetValue();
      selectedColor[0] = tmpColor[0];
      selectedColor[1] = tmpColor[1];
      selectedColor[2] = tmpColor[2];
      selectedColor[3] = 1.0f;
    }

    //check if there is an pointLineWidth property
    if (dynamic_cast<mitk::IntProperty*>(node->GetPropertyList(renderer)->GetProperty("point line width")) != NULL)
    {
      m_PointLineWidth = dynamic_cast<mitk::IntProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("point line width"))->GetValue();
    }
    else if (dynamic_cast<mitk::IntProperty*>(node->GetPropertyList(NULL)->GetProperty("point line width")) != NULL)
    {
      m_PointLineWidth = dynamic_cast<mitk::IntProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("point line width"))->GetValue();
    }

    //check if there is an point 2D size property
    if (dynamic_cast<mitk::IntProperty*>(node->GetPropertyList(renderer)->GetProperty("point 2D size")) != NULL)
    {
      m_Point2DSize = dynamic_cast<mitk::IntProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("point 2D size"))->GetValue();
    }
    else if (dynamic_cast<mitk::IntProperty*>(node->GetPropertyList(NULL)->GetProperty("point 2D size")) != NULL)
    {
      m_Point2DSize = dynamic_cast<mitk::IntProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("point 2D size"))->GetValue();
    }

    Point3D p;                      // currently visited point 
    Point3D lastP;                  // last visited point 
    Vector3D vec;                   // p - lastP
    Vector3D lastVec;               // lastP - point before lastP
    vec.Fill(0);

    mitk::Point3D projected_p;      // p projected on viewplane

    Point2D pt2d;       // projected_p in display coordinates
    Point2D lastPt2d;   // last projected_p in display coordinates
    Point2D preLastPt2d;// projected_p in display coordinates before lastPt2d

    while(it!=end) // iterate over all points
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

      //MouseOrientation
      bool isInputDevice=false;

      bool isRendererSlice = scalardiff < 0.00001; //cause roundoff error
      if(this->GetDataTreeNode()->GetBoolProperty("inputdevice",isInputDevice) && isInputDevice && !isRendererSlice )
      { 
        displayGeometry->Map(projected_p, pt2d);
        displayGeometry->WorldToDisplay(pt2d, pt2d);

        //Point size depending of distance to slice
        /*float p_size = (1/scalardiff)*10*m_Point2DSize;
        if(p_size < m_Point2DSize * 0.6 ) 
          p_size = m_Point2DSize * 0.6 ;
        else if ( p_size > m_Point2DSize )
          p_size = m_Point2DSize;*/
        float p_size = (1/scalardiff)*100.0;
        if(p_size < 6.0 ) 
          p_size = 6.0 ;
        else if ( p_size > 10.0 )
          p_size = 10.0;

        //draw Point
        float opacity = (p_size<8)?0.3:1.0;//don't get the opacity from the node? Feature not a bug! Otehrwise the 2D cross is hardly seen.
        glColor4f(unselectedColor[0],unselectedColor[1],unselectedColor[2],opacity);
        glPointSize(p_size);
        //glShadeModel(GL_FLAT);
        glBegin (GL_POINTS);
          glVertex2fv(&pt2d[0]);
        glEnd ();
      }

      //for point set
      if(!isInputDevice && ( (scalardiff<4.0) || (m_Polygon)))
      {
        Point2D tmp;
        displayGeometry->Map(projected_p, pt2d);
        displayGeometry->WorldToDisplay(pt2d, pt2d);

        Vector2D horz,vert;
        horz[0]=(float)m_Point2DSize-scalardiff*2; horz[1]=0;
        vert[0]=0;                vert[1]=(float)m_Point2DSize-scalardiff*2;

        // now paint text if available
        if (dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()
              ->GetProperty("label")) != NULL)
        {
          const char * pointLabel = dynamic_cast<mitk::StringProperty *>(
            this->GetDataTreeNode()->GetProperty("label"))->GetValue();
          char buffer[20];
          std::string l = pointLabel;
          if (input->GetSize()>1)
          {
            sprintf(buffer,"%d",j+1);
            l.append(buffer);
          }
          if (unselectedColor != NULL)
          {
            mitk::VtkPropRenderer* OpenGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
            float rgb[3];//yellow
            rgb[0] = unselectedColor[0]; rgb[1] = unselectedColor[1]; rgb[2] = unselectedColor[2];
            OpenGLrenderer->WriteSimpleText(l, pt2d[0] + text2dDistance, pt2d[1] + text2dDistance,rgb[0], rgb[1],rgb[2]);
          }
          else
          {
            mitk::VtkPropRenderer* OpenGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
            OpenGLrenderer->WriteSimpleText(l, pt2d[0] + text2dDistance, pt2d[1] + text2dDistance,0.0,1.0,0.0);
          }
        }
        
        if((m_ShowPoints) && (scalardiff<4.0))
        {
          //check if the point is to be marked as selected 
          if(selIt != selEnd || pointDataBroken)
          {
            bool addAsSelected = false;
            if (pointDataBroken)
              addAsSelected = false;
            else if (selIt->Value().selected)
              addAsSelected = true;
            else 
              addAsSelected = false;

            if (addAsSelected)
            {
              horz[0]=(float)m_Point2DSize;
              vert[1]=(float)m_Point2DSize;
              glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);
              glLineWidth(m_PointLineWidth);
              //a diamond around the point with the selected color
              glBegin (GL_LINE_LOOP);
               tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
               tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
               tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
               tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
              glEnd ();
              glLineWidth(1);
              //the actual point in the specified color to see the usual color of the point
              glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
              glPointSize(1);
              glBegin (GL_POINTS);
              tmp=pt2d;             glVertex2fv(&tmp[0]);
              glEnd ();
            }
            else //if not selected
            {
              glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
              glLineWidth(m_PointLineWidth);
              //drawing crosses
              glBegin (GL_LINES);
              tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
              tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
              tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
              tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
              glEnd ();
              glLineWidth(1);
            }
          }
        }

        bool drawLinesEtc = true;
        if (!m_ShowDistantLines && counter > 0) // check, whether this line should be drawn
        {
          ScalarType currentDistance = displayGeometry->GetWorldGeometry()->SignedDistance(p);
          ScalarType lastDistance =    displayGeometry->GetWorldGeometry()->SignedDistance(lastP);
          if ( currentDistance * lastDistance > 0.5 ) // points on same side of plane
            drawLinesEtc = false;
        }
 
        if ( m_Polygon && counter > 0 && drawLinesEtc) // draw a line
        {
          //get contour color property
          float contourColor[4] = {unselectedColor[0], unselectedColor[1], unselectedColor[2], unselectedColor[3]};//so if no property set, then use unselected color
          if (dynamic_cast<mitk::ColorProperty*>(node->GetPropertyList(renderer)->GetProperty("contourcolor")) != NULL)
          {
            mitk::Color tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("contourcolor"))->GetValue();
            contourColor[0] = tmpColor[0];
            contourColor[1] = tmpColor[1];
            contourColor[2] = tmpColor[2];
            contourColor[3] = 1.0f;
          }
          else if (dynamic_cast<mitk::ColorProperty*>(node->GetPropertyList(NULL)->GetProperty("contourcolor")) != NULL)
          {
            mitk::Color tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("contourcolor"))->GetValue();
            contourColor[0] = tmpColor[0];
            contourColor[1] = tmpColor[1];
            contourColor[2] = tmpColor[2];
            contourColor[3] = 1.0f;
          }
          //set this color
          glColor3f(contourColor[0],contourColor[1],contourColor[2]);

          glLineWidth( m_LineWidth );
          glBegin (GL_LINES);
          glVertex2fv(&pt2d[0]);
          glVertex2fv(&lastPt2d[0]);
          glEnd ();
          glLineWidth(1.0);
          if(m_ShowDistances) // calculate and print a distance
          {
            std::stringstream buffer;
            float distance = vec.GetNorm();
            buffer<<std::fixed <<std::setprecision(m_DistancesDecimalDigits)<<distance<<" mm";

            Vector2D vec2d = pt2d-lastPt2d;
            makePerpendicularVector2D(vec2d, vec2d);

            Vector2D pos2d = (lastPt2d.GetVectorFromOrigin()+pt2d)*0.5+vec2d*text2dDistance;

            mitk::VtkPropRenderer* OpenGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
            OpenGLrenderer->WriteSimpleText(buffer.str(), pos2d[0], pos2d[1]);
            //this->WriteTextXY(pos2d[0], pos2d[1], buffer.str(),renderer);
          }

          if(m_ShowAngles && counter > 1 ) // calculate and print the angle btw. two lines
          {
            std::stringstream buffer;
            //buffer << angle(vec.Get_vnl_vector(), -lastVec.Get_vnl_vector())*180/vnl_math::pi << "�";
            buffer << angle(vec.Get_vnl_vector(), -lastVec.Get_vnl_vector())*180/vnl_math::pi << (char)176;

            Vector2D vec2d = pt2d-lastPt2d;
            vec2d.Normalize();
            Vector2D lastVec2d = lastPt2d-preLastPt2d;
            lastVec2d.Normalize();
            vec2d=vec2d-lastVec2d;
            vec2d.Normalize();

            Vector2D pos2d = lastPt2d.GetVectorFromOrigin()+vec2d*text2dDistance*text2dDistance;

            mitk::VtkPropRenderer* OpenGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
            OpenGLrenderer->WriteSimpleText(buffer.str(), pos2d[0], pos2d[1]);
            //this->WriteTextXY(pos2d[0], pos2d[1], buffer.str(),renderer);
          }
      }
        counter++;
      }
      ++it;
      if(selIt != selEnd && !pointDataBroken)
        ++selIt;
      j++;
    }

    //recall the color to the same color before this drawing
    glColor3f(recallColor[0],recallColor[1],recallColor[2]);
  }
}

void mitk::PointSetMapper2D::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", mitk::IntProperty::New(2), renderer, overwrite ); // width of the line from one point to another
  node->AddProperty( "point line width", mitk::IntProperty::New(1), renderer, overwrite ); //width of the cross marking a point
  node->AddProperty( "point 2D size", mitk::IntProperty::New(8), renderer, overwrite ); // length of the cross marking a point // length of an edge of the box marking a point
  node->AddProperty( "show contour", mitk::BoolProperty::New(false), renderer, overwrite ); // contour of the line between points
  node->AddProperty( "show points", mitk::BoolProperty::New(true), renderer, overwrite ); //show or hide points 
  node->AddProperty( "show distances", mitk::BoolProperty::New(false), renderer, overwrite ); //show or hide distance measure (not always available)
  node->AddProperty( "distance decimal digits", mitk::IntProperty::New(2), renderer, overwrite ); //set the number of decimal digits to be shown
  node->AddProperty( "show angles", mitk::BoolProperty::New(false), renderer, overwrite ); //show or hide angle measurement (not always available)
  node->AddProperty( "show distant lines", mitk::BoolProperty::New(false), renderer, overwrite ); //show the line between to points from a distant view (equals "always on top" option)

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
