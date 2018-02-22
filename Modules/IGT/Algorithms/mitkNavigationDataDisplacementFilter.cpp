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

#include "mitkNavigationDataDisplacementFilter.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"

mitk::NavigationDataDisplacementFilter::NavigationDataDisplacementFilter()
: mitk::NavigationDataToNavigationDataFilter(), m_Transform6DOF(false)
{
  m_Offset[0] = 0.0;
  m_Offset[1] = 0.0;
  m_Offset[2] = 0.0;

  m_Transformation = mitk::NavigationData::New();

}


mitk::NavigationDataDisplacementFilter::~NavigationDataDisplacementFilter()
{
}


void mitk::NavigationDataDisplacementFilter::GenerateData()
{
  /* update outputs with tracking data from tools */
  if( !m_Transform6DOF )
  {
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
  else
  {
    if( this->GetNumberOfOutputs() < 2 )
    {
      MITK_WARN << "TrackedUltrasound not possible. The number of tracked devices must be at least 2.";
      return;
    }

    //important: First device = Needle | Second device = US-Tracker

    mitk::NavigationData::Pointer needleOut = this->GetOutput(0);
    const mitk::NavigationData* needleIn = this->GetInput(0);

    mitk::NavigationData::Pointer usTrackerOut = this->GetOutput(1);
    const mitk::NavigationData* usTrackerIn = this->GetInput(1);

    if(needleIn->IsDataValid() == false )
    {
      needleOut->SetDataValid(false);
    }
    else
      needleOut->Graft(needleIn);

    if (usTrackerIn->IsDataValid() == false)
    {
      usTrackerOut->SetDataValid(false);
    }
    else
      usTrackerOut->Graft(usTrackerIn);

    needleOut->Compose( usTrackerOut->GetInverse(), false );
    needleOut->Compose( m_Transformation->GetInverse() );

    usTrackerOut->SetDataValid(true);
    needleOut->SetDataValid(true);

    if( this->GetNumberOfOutputs() > 2 )
    {
      for( unsigned int i = 2; i < this->GetNumberOfOutputs(); ++i )
      {
        mitk::NavigationData* output = this->GetOutput(i);
        const mitk::NavigationData* input = this->GetInput(i);

        if (input->IsDataValid() == false)
        {
          output->SetDataValid(false);
          continue;
        }
        output->Graft(input);
        output->SetDataValid(true);
      }
    }
  }
}


void mitk::NavigationDataDisplacementFilter::SetTransformation(mitk::AffineTransform3D::Pointer transform)
{
  mitk::NavigationData::Pointer transformation = mitk::NavigationData::New(transform);
  m_Transformation = transformation;
}

void mitk::NavigationDataDisplacementFilter::SetParameters( const mitk::PropertyList* p )
{
  if (p == nullptr)
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
