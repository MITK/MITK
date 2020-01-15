/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRestorePlanePositionOperation.h"

namespace mitk
{
  RestorePlanePositionOperation::RestorePlanePositionOperation(
    OperationType operationType,
    ScalarType width,
    ScalarType height,
    Vector3D spacing,
    unsigned int pos,
    Vector3D direction,
    AffineTransform3D::Pointer transform /*, PlaneOrientation orientation*/)
    : Operation(operationType),
      m_Spacing(spacing),
      m_DirectionVector(direction),
      m_Width(width),
      m_Height(height),
      m_Pos(pos),
      m_Transform(transform) /*, m_Orientation(orientation)*/
  {
    // MITK_INFO<<"Width: "<<width<<"height: "<<height<<"spacing: "<<spacing<<"direction: "<<direction<<"transform:
    // "<<transform;
  }

  RestorePlanePositionOperation::~RestorePlanePositionOperation() {}
  Vector3D RestorePlanePositionOperation::GetSpacing() { return m_Spacing; }
  Vector3D RestorePlanePositionOperation::GetDirectionVector() { return m_DirectionVector; }
  ScalarType RestorePlanePositionOperation::GetWidth() { return m_Width; }
  ScalarType RestorePlanePositionOperation::GetHeight() { return m_Height; }
  unsigned int RestorePlanePositionOperation::GetPos() { return m_Pos; }
  AffineTransform3D::Pointer RestorePlanePositionOperation::GetTransform() { return m_Transform; }
} // namespace mitk
