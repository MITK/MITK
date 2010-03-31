/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataDisplacementFilter.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"

mitk::NavigationDataDisplacementFilter::NavigationDataDisplacementFilter() 
: mitk::NavigationDataToNavigationDataFilter()
{
  m_Offset[0] = 0.0;
  m_Offset[1] = 0.0;
  m_Offset[2] = 0.0;
}


mitk::NavigationDataDisplacementFilter::~NavigationDataDisplacementFilter()
{
}


void mitk::NavigationDataDisplacementFilter::GenerateData()
{
  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);

    if (input->IsDataValid() == false)
    {
      output->SetDataValid(false);
      continue;
    }
    output->Graft(input); // First, copy all information from input to output
    output->SetPosition(input->GetPosition() + m_Offset);  // Then change the member(s): add offset to position of navigation data
    output->SetDataValid(true); // operation was successful, therefore data of output is valid.
    // don't change anything else here
  }
}


void mitk::NavigationDataDisplacementFilter::SetParameters( const mitk::PropertyList* p )
{
  if (p == NULL)
    return;
  mitk::Vector3D v;
  if (p->GetPropertyValue<mitk::Vector3D>("NavigationDataDisplacementFilter_Offset", v) == true)  // search for Offset parameter
    this->SetOffset(v);   // apply if found;
}


mitk::PropertyList::ConstPointer mitk::NavigationDataDisplacementFilter::GetParameters() const
{
  mitk::PropertyList::Pointer p = mitk::PropertyList::New();
  p->SetProperty("NavigationDataDisplacementFilter_Offset", mitk::Vector3DProperty::New(this->GetOffset()));  // store Offset parameter
  return mitk::PropertyList::ConstPointer(p);
}
