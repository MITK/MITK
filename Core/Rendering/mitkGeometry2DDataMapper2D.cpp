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


#include <mitkGL.h>
#include "mitkGeometry2DDataMapper2D.h"
#include "mitkLine.h"
#include "mitkBaseRenderer.h"

#include "mitkPlaneGeometry.h"

#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkSmartPointerProperty.h"

#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkSurfaceMapper2D.h"

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
  return static_cast<const mitk::Geometry2DData * > ( GetData() );
}

void mitk::Geometry2DDataMapper2D::SetDataIteratorToOtherGeometry2Ds(const mitk::DataTreeIteratorBase* iterator)
{
  if(m_IteratorToOtherGeometry2Ds != iterator)
  {
    m_IteratorToOtherGeometry2Ds = iterator;
    Modified();
  }
}

void mitk::Geometry2DDataMapper2D::GenerateData()
{
  //collect all Geometry2DData's accessible by traversing m_IteratorToOtherGeometry2Ds
  m_OtherGeometry2Ds.clear();
  if(m_IteratorToOtherGeometry2Ds.IsNull()) return;

  mitk::DataTreeIteratorClone it = m_IteratorToOtherGeometry2Ds;
  while(!it->IsAtEnd())
  {
    if(it->Get().IsNotNull())
    {
      mitk::BaseData* data = it->Get()->GetData();
      if(data != NULL)
      {
        mitk::Geometry2DData* geometry2dData = dynamic_cast<mitk::Geometry2DData*>(data);
        if(geometry2dData!=NULL)
        {
          mitk::PlaneGeometry* planegeometry = dynamic_cast<mitk::PlaneGeometry*>(geometry2dData->GetGeometry2D());
          if(planegeometry!=NULL)
            m_OtherGeometry2Ds.push_back(it->Get());
        }
      }
    }
    ++it;
  }
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
      Point2D lineFrom, lineTo;
      bool intersecting = worldPlaneGeometry->IntersectWithPlane2D( input_planeGeometry, lineFrom, lineTo ) > 0;

      if(intersecting)
      {
        mitk::Line<ScalarType, 2> line, tmpLine;
        line.SetPoints(lineFrom, lineTo);

        displayGeometry->WorldToDisplay(lineFrom, lineFrom);
        displayGeometry->WorldToDisplay(lineTo, lineTo);
        Vector2D l;
        l=lineTo-lineFrom;
        ScalarType lengthInDisplayUnits = l.GetNorm();

        std::vector<ScalarType> lineParams;
        lineParams.reserve(m_OtherGeometry2Ds.size()+2);
        lineParams.push_back(0);
        lineParams.push_back(1);

        Vector2D d, dOrth;
        NodesVectorType::iterator otherPlanesIt = m_OtherGeometry2Ds.begin();
        NodesVectorType::iterator otherPlanesEnd = m_OtherGeometry2Ds.end();
        while(otherPlanesIt != otherPlanesEnd)
        {
          mitk::PlaneGeometry* otherPlane = static_cast<PlaneGeometry*>(static_cast<Geometry2DData*>((*otherPlanesIt)->GetData())->GetGeometry2D());
          if(otherPlane != input_planeGeometry)
          {
            bool otherIntersecting = worldPlaneGeometry->IntersectWithPlane2D( otherPlane, lineFrom, lineTo ) > 0;
            if(otherIntersecting)
            {
              tmpLine.SetPoints(lineFrom, lineTo);
              d = tmpLine.GetDirection();
              dOrth[0] = -d[1]; dOrth[1] = d[0];
              ScalarType t = (tmpLine.GetPoint1()-line.GetPoint1())*(dOrth);
              ScalarType norm = line.GetDirection()*dOrth;
              if(fabs(norm) > mitk::eps)
              {
                t /= norm;
                lineParams.push_back(t);
              }
            }
          }

          ++otherPlanesIt;
        }

        //apply color and opacity read from the PropertyList
        ApplyProperties(renderer);

        ScalarType gapSizeInPixel = 10;
        ScalarType gapSizeInParamUnits = 1.0/lengthInDisplayUnits*gapSizeInPixel;

        std::sort(lineParams.begin(), lineParams.end());

        Point2D p1, p2;
        ScalarType p1Param, p2Param; 
        
        p1Param = lineParams[0];
        p1 = line.GetPoint(p1Param);
        displayGeometry->WorldToDisplay(p1, p1);

        unsigned int i, preLastLineParam = lineParams.size()-1;
        for(i=1; i < preLastLineParam; ++i)
        {
          p2Param = lineParams[i]-gapSizeInParamUnits*0.5;
          p2 = line.GetPoint(p2Param);
          if(p2Param > p1Param)
          {
            //convert intersection points (until now mm) to display coordinates (units )
            displayGeometry->WorldToDisplay(p2, p2);

            //draw
            glBegin (GL_LINES);
            glVertex2f(p1[0],p1[1]);
            glVertex2f(p2[0],p2[1]);
            glEnd ();
          }
          p1Param = p2Param+gapSizeInParamUnits;
          p1 = line.GetPoint(p1Param);
          displayGeometry->WorldToDisplay(p1, p1);
        }
        //draw last line segment
        p2Param = lineParams[i];
        p2 = line.GetPoint(p2Param);
        displayGeometry->WorldToDisplay(p2, p2);
        glBegin (GL_LINES);
        glVertex2f(p1[0],p1[1]);
        glVertex2f(p2[0],p2[1]);
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
        surfaceCreator->PlaceByGeometryOn();
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
      m_SurfaceMapper->SetSurface(surfaceCreator->GetOutput());
      m_SurfaceMapper->SetDataTreeNode(GetDataTreeNode());
      
      m_SurfaceMapper->Paint(renderer);
    }
  }
}
