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

#include "mitkFeedbackContourTool.h"
#include "mitkToolManager.h"

#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"

#include "mitkDataStorage.h"
#include "mitkBaseRenderer.h"
#include "mitkContourUtils.h"
#include "mitkRenderingManager.h"

mitk::FeedbackContourTool::FeedbackContourTool(const char* type)
:SegTool2D(type),
 m_FeedbackContourVisible(false)
{
  m_FeedbackContour = ContourModel::New();
  m_FeedbackContour->SetClosed(true);
  m_FeedbackContourNode = DataNode::New();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
  m_FeedbackContourNode->SetName("feedback contour");
  m_FeedbackContourNode->SetProperty("visible", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("helper object", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("layer", IntProperty::New(1000));
  m_FeedbackContourNode->SetProperty("contour.project-onto-plane", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("contour.width", FloatProperty::New(2.0));

  this->SetFeedbackContourColorDefault();
}

mitk::FeedbackContourTool::~FeedbackContourTool()
{
}

void mitk::FeedbackContourTool::Activated()
{
  this->Disable3DRendering();

  DataStorage* storage = m_ToolManager->GetDataStorage();
  assert (storage);
  storage->Add( m_FeedbackContourNode );

  Superclass::Activated();
}

void mitk::FeedbackContourTool::Deactivated()
{
  DataStorage* storage = m_ToolManager->GetDataStorage();
  assert (storage);
  storage->Remove( m_FeedbackContourNode );

  Superclass::Deactivated();
}

void mitk::FeedbackContourTool::SetFeedbackContourColor( float r, float g, float b )
{
  m_FeedbackContourNode->SetProperty("contour.color", ColorProperty::New(r, g, b));
}

void mitk::FeedbackContourTool::SetFeedbackContourColorDefault()
{
  m_FeedbackContourNode->SetProperty("contour.color", ColorProperty::New(0.0/255.0, 255.0/255.0, 0.0/255.0));
}

mitk::ContourModel* mitk::FeedbackContourTool::GetFeedbackContour()
{
  return m_FeedbackContour;
}

void mitk::FeedbackContourTool::SetFeedbackContour(ContourModel& contour)
{
  m_FeedbackContour = &contour;
  m_FeedbackContour->Modified();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
}

void mitk::FeedbackContourTool::SetFeedbackContourVisible(bool visible)
{
  if ( m_FeedbackContourVisible == visible )
    return; // nothing changed

  m_FeedbackContourVisible = visible;

  m_FeedbackContourNode->SetVisibility(m_FeedbackContourVisible);
}

void mitk::FeedbackContourTool::Disable3DRendering()
{
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
}
