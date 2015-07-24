
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
mitk::ExampleDataStructure::ExampleDataStructure()
: m_Data("Initialized")
{
  this->SetGeometry(mitk::Geometry3D::New());
}

mitk::ExampleDataStructure::~ExampleDataStructure()
{
}

void mitk::ExampleDataStructure::AppendAString(const std::string input)
{
  m_Data.append( input );
}


bool mitk::Equal( mitk::ExampleDataStructure* leftHandSide, mitk::ExampleDataStructure* rightHandSide, mitk::ScalarType /*eps*/, bool verbose )
{
  bool noDifferenceFound = true;

    if( rightHandSide == NULL )
  {
    if(verbose)
    {
      MITK_INFO << "[Equal( ExampleDataStructure*, ExampleDataStructure* )] rightHandSide NULL.";
    }
    return false;
  }

  if( leftHandSide == NULL )
  {
    if(verbose)
    {
      MITK_INFO << "[Equal( ExampleDataStructure*, ExampleDataStructure* )] leftHandSide NULL.";
    }
    return false;
  }

  if (!(leftHandSide->GetData() == rightHandSide->GetData()) )
    {
      if(verbose)
      {
        MITK_INFO << "[Equal( ExampleDataStructure*, ExampleDataStructure* )] Data not equal. ";
        MITK_INFO << leftHandSide->GetData() << " != " << rightHandSide->GetData();
      }
      noDifferenceFound = false;
    }

  return noDifferenceFound;
}
