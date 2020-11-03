
/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExampleDataStructure.h"
#include "mitkGeometry3D.h"

// implementation of virtual methods

void mitk::ExampleDataStructure::UpdateOutputInformation()
{
}

void mitk::ExampleDataStructure::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::ExampleDataStructure::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::ExampleDataStructure::VerifyRequestedRegion()
{
  return true;
}

void mitk::ExampleDataStructure::SetRequestedRegion(const itk::DataObject *)
{
}

/* Constructor and Destructor */
mitk::ExampleDataStructure::ExampleDataStructure() : m_Data("Initialized")
{
  this->SetGeometry(mitk::Geometry3D::New());
}

mitk::ExampleDataStructure::~ExampleDataStructure()
{
}

void mitk::ExampleDataStructure::AppendAString(const std::string input)
{
  m_Data.append(input);
}

bool mitk::Equal(mitk::ExampleDataStructure *leftHandSide,
                 mitk::ExampleDataStructure *rightHandSide,
                 mitk::ScalarType /*eps*/,
                 bool verbose)
{
  bool noDifferenceFound = true;

  if (rightHandSide == nullptr)
  {
    if (verbose)
    {
      MITK_INFO << "[Equal( ExampleDataStructure*, ExampleDataStructure* )] rightHandSide nullptr.";
    }
    return false;
  }

  if (leftHandSide == nullptr)
  {
    if (verbose)
    {
      MITK_INFO << "[Equal( ExampleDataStructure*, ExampleDataStructure* )] leftHandSide nullptr.";
    }
    return false;
  }

  if (!(leftHandSide->GetData() == rightHandSide->GetData()))
  {
    if (verbose)
    {
      MITK_INFO << "[Equal( ExampleDataStructure*, ExampleDataStructure* )] Data not equal. ";
      MITK_INFO << leftHandSide->GetData() << " != " << rightHandSide->GetData();
    }
    noDifferenceFound = false;
  }

  return noDifferenceFound;
}
