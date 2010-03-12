/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkFeedbackContourTool.h"
#include "mitkToolManager.h"

#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"

#include "mitkDataStorage.h"
#include "mitkBaseRenderer.h"

#include "mitkRenderingManager.h"

mitk::FeedbackContourTool::FeedbackContourTool(const char* type)
:SegTool2D(type),
 m_FeedbackContourVisible(false),
 m_ContourUtils(ContourUtils::New())
{
  m_FeedbackContour = Contour::New();
  m_FeedbackContourNode = DataNode::New();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
  m_FeedbackContourNode->SetProperty("name", StringProperty::New("One of FeedbackContourTool's feedback nodes"));
  m_FeedbackContourNode->SetProperty("visible", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("helper object", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("layer", IntProperty::New(1000));
  m_FeedbackContourNode->SetProperty("project", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("Width", FloatProperty::New(1)); // uppercase! Slim line looks very accurate :-)

  // set explicitly visible=false for all 3D renderer (that exist already ...)
  const RenderingManager::RenderWindowVector& renderWindows = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  for (RenderingManager::RenderWindowVector::const_iterator iter = renderWindows.begin();
       iter != renderWindows.end();
       ++iter)
  {
    if ( mitk::BaseRenderer::GetInstance((*iter))->GetMapperID() == BaseRenderer::Standard3D )
    //if ( (*iter)->GetRenderer()->GetMapperID() == BaseRenderer::Standard3D )
    {
      m_FeedbackContourNode->SetProperty("visible", BoolProperty::New(false), mitk::BaseRenderer::GetInstance((*iter)));
    }
  }

  SetFeedbackContourColorDefault();
}

mitk::FeedbackContourTool::~FeedbackContourTool()
{
}

void mitk::FeedbackContourTool::SetFeedbackContourColor( float r, float g, float b )
{
  m_FeedbackContourNode->SetProperty("color", ColorProperty::New(r, g, b));
}

void mitk::FeedbackContourTool::SetFeedbackContourColorDefault()
{
  m_FeedbackContourNode->SetProperty("color", ColorProperty::New(0.0/255.0, 255.0/255.0, 0.0/255.0));
}

mitk::Contour* mitk::FeedbackContourTool::GetFeedbackContour()
{
  return m_FeedbackContour;
}

void mitk::FeedbackContourTool::SetFeedbackContour(Contour& contour)
{
  m_FeedbackContour = &contour;
  m_FeedbackContourNode->SetData( m_FeedbackContour );
}

void mitk::FeedbackContourTool::SetFeedbackContourVisible(bool visible)
{
  if ( m_FeedbackContourVisible == visible ) return; // nothing changed

  if ( DataStorage* storage = m_ToolManager->GetDataStorage() )
  {
    if (visible)
    {
      storage->Add( m_FeedbackContourNode );
    }
    else
    {
      storage->Remove( m_FeedbackContourNode );
    }
  }

  m_FeedbackContourVisible = visible;
}

mitk::Contour::Pointer mitk::FeedbackContourTool::ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool correctionForIpSegmentation, bool constrainToInside)
{
  return m_ContourUtils->ProjectContourTo2DSlice(slice, contourIn3D, correctionForIpSegmentation, constrainToInside);
}
    
mitk::Contour::Pointer mitk::FeedbackContourTool::BackProjectContourFrom2DSlice(Image* slice, Contour* contourIn2D, bool correctionForIpSegmentation)
{
  return m_ContourUtils->BackProjectContourFrom2DSlice(slice, contourIn2D, correctionForIpSegmentation);
}

void mitk::FeedbackContourTool::FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  m_ContourUtils->FillContourInSlice(projectedContour, sliceImage, paintingPixelValue);
}

