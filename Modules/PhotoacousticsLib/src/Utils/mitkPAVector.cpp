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

#include "mitkPAVector.h"
#include "chrono"
#include <cmath>
mitk::pa::Vector::Vector()
{
  m_Vector.Fill(0);
}

mitk::pa::Vector::~Vector()
{
  m_Vector.Fill(0);
}

double mitk::pa::Vector::GetNorm()
{
  return m_Vector.GetNorm();
}

double mitk::pa::Vector::GetElement(unsigned short index)
{
  return m_Vector.GetElement(index);
}

void mitk::pa::Vector::SetElement(unsigned short index, double value)
{
  m_Vector.SetElement(index, value);
}

void mitk::pa::Vector::Normalize()
{
  double norm = m_Vector.GetNorm();
  m_Vector.SetElement(0, m_Vector.GetElement(0) / norm);
  m_Vector.SetElement(1, m_Vector.GetElement(1) / norm);
  m_Vector.SetElement(2, m_Vector.GetElement(2) / norm);
}

void mitk::pa::Vector::SetValue(mitk::pa::Vector::Pointer value)
{
  m_Vector.SetElement(0, value->GetElement(0));
  m_Vector.SetElement(1, value->GetElement(1));
  m_Vector.SetElement(2, value->GetElement(2));
}

void mitk::pa::Vector::RandomizeByPercentage(double percentage, double bendingFactor, std::mt19937* rng)
{
  std::uniform_real_distribution<> range(-percentage, percentage);
  m_Vector.SetElement(0, m_Vector.GetElement(0) + (bendingFactor * range(*rng)));
  m_Vector.SetElement(1, m_Vector.GetElement(1) + (bendingFactor * range(*rng)));
  m_Vector.SetElement(2, m_Vector.GetElement(2) + (bendingFactor * range(*rng)));
}

void mitk::pa::Vector::Randomize(double xLowerLimit, double xUpperLimit, double yLowerLimit, double yUpperLimit, double zLowerLimit, double zUpperLimit, std::mt19937* rng)
{
  std::uniform_real_distribution<> rangeX(xLowerLimit, xUpperLimit);
  std::uniform_real_distribution<> rangeY(yLowerLimit, yUpperLimit);
  std::uniform_real_distribution<> rangeZ(zLowerLimit, zUpperLimit);
  m_Vector.SetElement(0, rangeX(*rng));
  m_Vector.SetElement(1, rangeY(*rng));
  m_Vector.SetElement(2, rangeZ(*rng));
}

void mitk::pa::Vector::Randomize(double xLimit, double yLimit, double zLimit, std::mt19937* rng)
{
  Randomize(0, xLimit, 0, yLimit, 0, zLimit, rng);
}

void mitk::pa::Vector::Randomize(std::mt19937* rng)
{
  Randomize(-1, 1, -1, 1, -1, 1, rng);
}

void mitk::pa::Vector::PrintSelf(std::ostream& os, itk::Indent /*indent*/) const
{
  os << "X: " << m_Vector.GetElement(0) << std::endl;
  os << "Y: " << m_Vector.GetElement(1) << std::endl;
  os << "Z: " << m_Vector.GetElement(2) << std::endl;
  os << "Length: " << m_Vector.GetNorm() << std::endl;
}

void mitk::pa::Vector::Rotate(double thetaChange, double phiChange)
{
  MITK_DEBUG << "Vector before rotation: (" << GetElement(0) << "|" << GetElement(1) << "|" << GetElement(2) << ")";
  if (thetaChange == 0 && phiChange == 0)
    return;

  double x = GetElement(0);
  double y = GetElement(1);
  double z = GetElement(2);

  double r = sqrt(x*x + y*y + z*z);
  if (r == 0)
    return;

  double theta = acos(z / r);
  double phi = atan2(y, x);

  theta += thetaChange;
  phi += phiChange;

  SetElement(0, r * sin(theta) * cos(phi));
  SetElement(1, r * sin(theta) * sin(phi));
  SetElement(2, r * cos(theta));

  MITK_DEBUG << "Vector after rotation: (" << GetElement(0) << "|" << GetElement(1) << "|" << GetElement(2) << ")";
}

void mitk::pa::Vector::Scale(double factor)
{
  m_Vector.SetElement(0, m_Vector.GetElement(0)*factor);
  m_Vector.SetElement(1, m_Vector.GetElement(1)*factor);
  m_Vector.SetElement(2, m_Vector.GetElement(2)*factor);
}

mitk::pa::Vector::Pointer mitk::pa::Vector::Clone()
{
  auto returnVector = Vector::New();
  returnVector->SetElement(0, this->GetElement(0));
  returnVector->SetElement(1, this->GetElement(1));
  returnVector->SetElement(2, this->GetElement(2));
  return returnVector;
}

bool mitk::pa::Equal(const Vector::Pointer leftHandSide, const Vector::Pointer rightHandSide, double eps, bool verbose)
{
  MITK_INFO(verbose) << "=== mitk::pa::Vector Equal ===";

  if (rightHandSide.IsNull() || leftHandSide.IsNull())
  {
    MITK_INFO(verbose) << "Cannot compare nullpointers";
    return false;
  }

  if (leftHandSide->GetElement(0) - rightHandSide->GetElement(0) > eps)
  {
    MITK_INFO(verbose) << "Element[0] not equal";
    return false;
  }

  if (leftHandSide->GetElement(1) - rightHandSide->GetElement(1) > eps)
  {
    MITK_INFO(verbose) << "Element[1] not equal";
    return false;
  }

  if (leftHandSide->GetElement(2) - rightHandSide->GetElement(2) > eps)
  {
    MITK_INFO(verbose) << "Element[2] not equal";
    return false;
  }

  return true;
}
