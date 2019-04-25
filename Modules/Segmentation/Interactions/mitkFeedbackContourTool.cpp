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
#include <mitkContourModelGLMapper2D.h>
#include <mitkVtkGLMapperWrapper.h>

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
 m_FeedbackContourVisible(false)
{
  m_FeedbackContour = ContourModel::New();
  m_FeedbackContour->SetClosed(true);
  m_FeedbackContourNode = DataNode::New();
  m_FeedbackContourNode->SetData( m_FeedbackContour );
  m_FeedbackContourNode->SetProperty("name", StringProperty::New("One of FeedbackContourTool's feedback nodes"));
  m_FeedbackContourNode->SetProperty("visible", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("helper object", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("layer", IntProperty::New(1000));
  m_FeedbackContourNode->SetProperty("fixedLayer", BoolProperty::New(true));
  m_FeedbackContourNode->SetProperty("contour.project-onto-plane", BoolProperty::New(false));
  m_FeedbackContourNode->SetProperty("contour.width", FloatProperty::New(1.0));
  m_FeedbackContourNode->SetProperty("draw-in-3d", BoolProperty::New(false));
  m_FeedbackContourNode->SetProperty("contour.check-distance", BoolProperty::New(false));

  m_FeedbackContourNode->SetMapper(mitk::BaseRenderer::Standard3D,
      mitk::VtkGLMapperWrapper::New(mitk::ContourModelGLMapper2D::New().GetPointer()));

  SetFeedbackContourColorDefault();
}

mitk::FeedbackContourTool::~FeedbackContourTool()
{
}

void mitk::FeedbackContourTool::SetFeedbackContourColor( float r, float g, float b )
{
  m_FeedbackContourNode->SetProperty("contour.color", ColorProperty::New(r, g, b));
}

void mitk::FeedbackContourTool::SetFeedbackContourColorDefault()
{
    m_FeedbackContourNode->SetProperty("contour.color", ColorProperty::New(0.0/255.0, 255.0/255.0, 0.0/255.0));
}

void mitk::FeedbackContourTool::SetFeedbackContourDraw3D(bool draw3D)
{
  m_FeedbackContourNode->SetProperty("draw-in-3d", BoolProperty::New(draw3D));
}

void mitk::FeedbackContourTool::Deactivated()
{
  Superclass::Deactivated();
  if (m_FeedbackContourNode.IsNotNull()) {
    m_FeedbackContour->Clear();
    SetFeedbackContourVisible(false);
  }
}

void mitk::FeedbackContourTool::Activated()
{
  Superclass::Activated();
  SetFeedbackContourVisible(true);
}

mitk::ContourModel* mitk::FeedbackContourTool::GetFeedbackContour()
{
  return m_FeedbackContour;
}

void mitk::FeedbackContourTool::SetFeedbackContour(ContourModel::Pointer contour)
{
  m_FeedbackContour = contour;
  m_FeedbackContourNode->SetData(m_FeedbackContour);
}

void mitk::FeedbackContourTool::SetFeedbackContourVisible(bool visible)
{
  if ( m_FeedbackContourVisible == visible ) {
    return; // nothing changed
  }

  if ( DataStorage* storage = m_ToolManager->GetDataStorage()) {
    if (visible) {
      storage->Add( m_FeedbackContourNode );
    } else {
      storage->Remove( m_FeedbackContourNode );
    }
  }

  m_FeedbackContourVisible = visible;
}

mitk::ContourModel::Pointer mitk::FeedbackContourTool::ProjectContourTo2DSlice(Image* slice, ContourModel* contourIn3D, bool correctionForIpSegmentation, bool constrainToInside)
{
  return mitk::ContourModelUtils::ProjectContourTo2DSlice(slice, contourIn3D, correctionForIpSegmentation, constrainToInside);
}

mitk::ContourModel::Pointer mitk::FeedbackContourTool::BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, ContourModel* contourIn2D, bool correctionForIpSegmentation)
{
  return mitk::ContourModelUtils::BackProjectContourFrom2DSlice(sliceGeometry, contourIn2D, correctionForIpSegmentation);
}

void mitk::FeedbackContourTool::FillContourInSlice( ContourModel* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  this->FillContourInSlice(projectedContour, 0, sliceImage, paintingPixelValue);
}

void mitk::FeedbackContourTool::FillContourInSlice( ContourModel* projectedContour, unsigned int timeStep, Image* sliceImage, int paintingPixelValue )
{
  mitk::ContourModelUtils::FillContourInSlice(projectedContour, timeStep, sliceImage, paintingPixelValue);
}
