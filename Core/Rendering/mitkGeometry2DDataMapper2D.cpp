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

#include <mitkGL.h>
#include "mitkGeometry2DDataMapper2D.h"

#include "mitkBaseRenderer.h"

#include "mitkPlaneGeometry.h"

#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkSmartPointerProperty.h"

#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkSurfaceMapper2D.h"

#include "GL/glu.h"

//##ModelId=3E639E100243
mitk::Geometry2DDataMapper2D::Geometry2DDataMapper2D() : m_SurfaceMapper(NULL)
{
}

//##ModelId=3E639E100257
mitk::Geometry2DDataMapper2D::~Geometry2DDataMapper2D()
{
}

//##ModelId=3E6423D20341
const mitk::Geometry2DData *mitk::Geometry2DDataMapper2D::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }
  
  return static_cast<const mitk::Geometry2DData * > ( GetData() );
}


//##ModelId=3E67D77A0109
void mitk::Geometry2DDataMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) return;
  
  //	@FIXME: Logik fuer update
  bool updateNeccesary=true;
  
  if (updateNeccesary) {
    // ok, das ist aus GenerateData kopiert
    
    mitk::Geometry2DData::Pointer input  = const_cast<mitk::Geometry2DData*>(this->GetInput());
    
    //intersecting with ourself?
    if(input->GetGeometry2D()==renderer->GetCurrentWorldGeometry2D())
      return; //do nothing!
    
    PlaneGeometry::ConstPointer input_planeGeometry = dynamic_cast<const PlaneGeometry *>(input->GetGeometry2D());
    PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const PlaneGeometry*>(renderer->GetCurrentWorldGeometry2D());        
    
    if(worldPlaneGeometry.IsNotNull() && (input_planeGeometry.IsNotNull()))
    {
      mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
      assert(displayGeometry);
      
      //calculate intersection of the plane data with the border of the world geometry rectangle
      //float aaaa[2]={10,11};
      Point2D lineFrom;// =(float[2]){10,11}; 
      Point2D lineTo;
      bool intersecting = worldPlaneGeometry->IntersectWithPlane2D( input_planeGeometry, lineFrom, lineTo );
      
      if(intersecting)
      {
        //convert intersection points (until now mm) to display coordinates (units )
        displayGeometry->MMToDisplay(lineFrom, lineFrom);
        displayGeometry->MMToDisplay(lineTo, lineTo);
        
        //convert display coordinates ( (0,0) is top-left ) in GL coordinates ( (0,0) is bottom-left )
        //                float toGL=//displayGeometry->GetDisplayHeight(); displayGeometry->GetCurrentWorldGeometry2D()->GetHeightInUnits();
        //                lineFrom[1]=toGL-lineFrom[1];
        //                lineTo[1]=toGL-lineTo[1];
        
        //apply color and opacity read from the PropertyList
        ApplyProperties(renderer);

        //draw
        glBegin (GL_LINE_LOOP);
        glVertex2f(lineFrom[0],lineFrom[1]);
        glVertex2f(lineTo[0],lineTo[1]);
        glVertex2f(lineFrom[0],lineFrom[1]);
        glEnd ();
      }
    }
    else
    {
      mitk::Geometry2DDataToSurfaceFilter::Pointer surfaceCreator;
      mitk::SmartPointerProperty::Pointer surfacecreatorprop;
      surfacecreatorprop=dynamic_cast<mitk::SmartPointerProperty*>(GetDataTreeNode()->GetProperty("surfacegeometry", renderer).GetPointer());
      if( (surfacecreatorprop.IsNull()) || 
          (surfacecreatorprop->GetSmartPointer().IsNull()) ||
          ((surfaceCreator=dynamic_cast<mitk::Geometry2DDataToSurfaceFilter*>(surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull())
        )
      {
        surfaceCreator = mitk::Geometry2DDataToSurfaceFilter::New();
        surfacecreatorprop=new mitk::SmartPointerProperty(surfaceCreator);
        GetDataTreeNode()->SetProperty("surfacegeometry", surfacecreatorprop);
      }
      
      surfaceCreator->SetInput(input);
      
      int res;
      bool usegeometryparametricbounds=true;
      if(GetDataTreeNode()->GetIntProperty("xresolution", res, renderer))
      {
        surfaceCreator->SetXResolution(res);
        usegeometryparametricbounds=false;        
      }
      if(GetDataTreeNode()->GetIntProperty("yresolution", res, renderer))
      {
        surfaceCreator->SetYResolution(res);
        usegeometryparametricbounds=false;        
      }
      surfaceCreator->SetUseGeometryParametricBounds(usegeometryparametricbounds);

      surfaceCreator->Update(); //@FIXME ohne das crash
      
      if(m_SurfaceMapper.IsNull())
        m_SurfaceMapper=mitk::SurfaceMapper2D::New();
      m_SurfaceMapper->SetInput(GetDataTreeNode());
      m_SurfaceMapper->SetSurface(surfaceCreator->GetOutput());
      
      m_SurfaceMapper->Paint(renderer);
    }
  }
}

//##ModelId=3E67E1B90237
void mitk::Geometry2DDataMapper2D::Update()
{
}

//##ModelId=3E67E285024E
void mitk::Geometry2DDataMapper2D::GenerateOutputInformation()
{
}
