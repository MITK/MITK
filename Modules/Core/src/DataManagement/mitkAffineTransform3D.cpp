/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAffineTransform3D.h>

void mitk::ToJSON(nlohmann::json& j, AffineTransform3D::ConstPointer transform)
{
  const auto& matrix = transform->GetMatrix().GetVnlMatrix();

  for (int row = 0; row < 3; ++row)
  {
    for (int column = 0; column < 3; ++column)
    {
      j.push_back(matrix[row][column]);
    }

    j.push_back(0);
  }

  const auto& offset = transform->GetOffset();

  for (int column = 0; column < 3; ++column)
    j.push_back(offset[column]);

  j.push_back(1);
}

void mitk::FromJSON(const nlohmann::json& j, AffineTransform3D::Pointer transform)
{
  size_t i = 0;
  itk::Matrix<ScalarType, 3, 3> matrix;

  for (int row = 0; row < 3; ++row)
  {
    for (int column = 0; column < 3; ++column)
      j.at(i++).get_to(matrix[row][column]);

    ++i;
  }

  transform->SetMatrix(matrix);

  itk::Vector<ScalarType, 3> offset;

  for (int column = 0; column < 3; ++column)
    j.at(i++).get_to(offset[column]);

  transform->SetOffset(offset);
}
