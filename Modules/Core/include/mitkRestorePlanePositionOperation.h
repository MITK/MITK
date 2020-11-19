/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestorePlanePositionOperation_h_Included
#define mitkRestorePlanePositionOperation_h_Included

#include "mitkCommon.h"
#include "mitkNumericTypes.h"
#include "mitkPointOperation.h"

namespace mitk
{
  class MITKCORE_EXPORT RestorePlanePositionOperation : public Operation
  {
  public:
    RestorePlanePositionOperation(OperationType operationType,
                                  ScalarType width,
                                  ScalarType height,
                                  Vector3D spacing,
                                  unsigned int pos,
                                  Vector3D direction,
                                  AffineTransform3D::Pointer transform);

    ~RestorePlanePositionOperation() override;

    Vector3D GetDirectionVector();

    ScalarType GetWidth();

    ScalarType GetHeight();

    Vector3D GetSpacing();

    unsigned int GetPos();

    AffineTransform3D::Pointer GetTransform();

  private:
    Vector3D m_Spacing;

    Vector3D m_DirectionVector;

    ScalarType m_Width;

    ScalarType m_Height;

    unsigned int m_Pos;

    AffineTransform3D::Pointer m_Transform;
  };
} // namespace mitk
#endif
