/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataTransformFilter.h"


mitk::NavigationDataTransformFilter::NavigationDataTransformFilter()
: mitk::NavigationDataToNavigationDataFilter()
{
  m_Rigid3DTransform = nullptr;
  m_Precompose = false;
}


mitk::NavigationDataTransformFilter::~NavigationDataTransformFilter()
{
  m_Rigid3DTransform = nullptr;
}

void mitk::NavigationDataTransformFilter::GenerateData()
{

  // only update data if m_Transform was set
  if(m_Rigid3DTransform.IsNull())
  {
    itkExceptionMacro("Invalid parameter: Transform was not set!  Use SetRigid3DTransform() before updating the filter.");
    return;
  }
  else
  {
    this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

    /* update outputs with tracking data from tools */
    for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)
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

      // Get input position/orientation as float numbers
      const NavigationData::PositionType    pInF = input->GetPosition();
      const NavigationData::OrientationType oInF = input->GetOrientation();

      // Cast the input NavigationData to double precision
      TransformType::OutputVectorType pInD;
      FillVector3D(pInD, pInF[0], pInF[1], pInF[2]);
      TransformType::VersorType oInD;
      oInD.Set(oInF.x(), oInF.y(), oInF.z(), oInF.r());

      TransformType::Pointer composedTransform = TransformType::New();
      // SetRotation+SetOffset defines the Tip-to-World coordinate frame
      // transformation ("World" is used in the generic sense)
      composedTransform->SetRotation(oInD);
      composedTransform->SetOffset(pInD);
      // If !m_Precompose: The resulting transform is Tip-to-UserWorld
      // If m_Precompose:  The resulting transform is UserTip-to-World
      composedTransform->Compose(m_Rigid3DTransform, m_Precompose);

      // Transformed position/orientation as double numbers
      const TransformType::OutputVectorType  pOutD = composedTransform->GetOffset();
      const TransformType::VersorType        oOutD = composedTransform->GetVersor();

      // Cast to transformed NavigationData back to float precision
      NavigationData::OrientationType oOutF(oOutD.GetX(), oOutD.GetY(), oOutD.GetZ(), oOutD.GetW());
      NavigationData::PositionType pOutF;
      FillVector3D(pOutF, pOutD[0], pOutD[1], pOutD[2]);

      output->SetOrientation(oOutF);
      output->SetPosition(pOutF);
      output->SetDataValid(true); // operation was successful, therefore data of output is valid.
    }
  }
}
