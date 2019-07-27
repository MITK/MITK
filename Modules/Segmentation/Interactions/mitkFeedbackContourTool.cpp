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

  m_FeedbackContourNode->SetMapper(mitk::BaseRenderer::Standard3D, mitk::ContourModelGLMapper2D::New().GetPointer());

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

void mitk::FeedbackContourTool::resampleDecomposition(mitk::Image* image, mitk::Image* plane)
{
  m_PlaneUpDecomposition.Fill(0.);
  m_PlaneRightDecomposition.Fill(0.);

  mitk::Vector3D imageSpacing = image->GetGeometry()->GetSpacing();
  //std::cout << "Image Spacing: " << imageSpacing << "\n";

  mitk::Vector3D imageNormX = image->GetGeometry()->GetAxisVector(0);
  imageNormX.Normalize();
  mitk::Vector3D imageX = imageNormX * imageSpacing[0];

  mitk::Vector3D imageNormY = image->GetGeometry()->GetAxisVector(1);
  imageNormY.Normalize();
  mitk::Vector3D imageY = imageNormY * imageSpacing[1];

  mitk::Vector3D imageNormZ = image->GetGeometry()->GetAxisVector(2);
  imageNormZ.Normalize();
  mitk::Vector3D imageZ = imageNormZ * imageSpacing[2];

  //std::cout << "Image X: " << imageX << "\n";
  //std::cout << "Image Y: " << imageY << "\n";
  //std::cout << "Image Z: " << imageZ << "\n";

  mitk::Vector3D planeSpacing = plane->GetGeometry()->GetSpacing();
  //std::cout << "Plane Spacing: " << planeSpacing << "\n";

  mitk::Vector3D temp;
  temp = plane->GetGeometry()->GetAxisVector(0);
  temp.Normalize();
  mitk::Vector3D planeX = temp * planeSpacing[0];
  temp = plane->GetGeometry()->GetAxisVector(1);
  temp.Normalize();
  mitk::Vector3D planeY = temp * planeSpacing[1];
  //std::cout << "Plane X: " << planeX << "\n";
  //std::cout << "Plane Y: " << planeY << "\n";

  mitk::Vector3D projectionRight;
  projectionRight[0] = projectRatio(imageX, planeX);
  projectionRight[1] = projectRatio(imageY, planeX);
  projectionRight[2] = projectRatio(imageZ, planeX);

  mitk::Vector3D projectionUp;
  projectionUp[0] = projectRatio(imageX, planeY);
  projectionUp[1] = projectRatio(imageY, planeY);
  projectionUp[2] = projectRatio(imageZ, planeY);

  for (int i = 0; i < 3; i++) {
    m_PlaneRightDecomposition[i] = (projectionRight[0] * imageNormX[i] + projectionRight[1] * imageNormY[i] + projectionRight[2] * imageNormZ[i]) * planeSpacing[0];
    m_PlaneUpDecomposition[i]    = (projectionUp[0] * imageNormX[i] + projectionUp[1] * imageNormY[i] + projectionUp[2] * imageNormZ[i]) * planeSpacing[1];
  }

  //std::cout << "Plane Right: " << m_PlaneRightDecomposition << "\n";
  //std::cout << "Plane Up: " << m_PlaneUpDecomposition << "\n\n";
}

mitk::Vector3D::ValueType mitk::FeedbackContourTool::projectRatio(mitk::Vector3D a, mitk::Vector3D b)
{
  // operator* means dot for itk vectors cause reasons
  return (a * b) / (b.GetSquaredNorm());
}

void mitk::FeedbackContourTool::offsetPointForContour(Point3D& worldPoint, Point3D planeOffset)
{
  worldPoint[0] += planeOffset[0] * m_PlaneRightDecomposition[0];
  worldPoint[1] += planeOffset[0] * m_PlaneRightDecomposition[1];
  worldPoint[2] += planeOffset[0] * m_PlaneRightDecomposition[2];

  worldPoint[0] += planeOffset[1] * m_PlaneUpDecomposition[0];
  worldPoint[1] += planeOffset[1] * m_PlaneUpDecomposition[1];
  worldPoint[2] += planeOffset[1] * m_PlaneUpDecomposition[2];
}
