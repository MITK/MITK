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


#include "mitkDoseNodeHelper.h"

#include <mitkRTConstants.h>
#include <mitkIsoDoseLevelCollections.h>
#include <mitkIsoDoseLevelSetProperty.h>
#include <mitkIsoDoseLevelVectorProperty.h>
#include <mitkDoseImageVtkMapper2D.h>
#include <mitkIsoLevelsGenerator.h>

#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingModeProperty.h>

void mitk::ConfigureNodeAsDoseNode(mitk::DataNode* node, mitk::DoseValueAbs referenceDose)
{
  if (node)
  {
    //set some specific colorwash and isoline properties
    node->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), true);
    node->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), false);

    node->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);

    mitk::IsoDoseLevelSet::Pointer isoDoseLevelPreset = mitk::GeneratIsoLevels_Virtuos();
    mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(isoDoseLevelPreset);

    node->SetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(), levelSetProp);

    mitk::IsoDoseLevelVector::Pointer levelVector = mitk::IsoDoseLevelVector::New();
    mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(levelVector);
    node->SetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(), levelVecProp);

    mitk::RenderingModeProperty::Pointer renderingModeProp = mitk::RenderingModeProperty::New();

      //Generating the Colorwash
      vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

      for (mitk::IsoDoseLevelSet::ConstIterator itIsoDoseLevel = isoDoseLevelPreset->Begin(); itIsoDoseLevel != isoDoseLevelPreset->End(); ++itIsoDoseLevel)
      {
        float *hsv = new float[3];
        //used for transfer rgb to hsv
        vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();
        if (itIsoDoseLevel->GetVisibleColorWash())
        {
          cCalc->RGBToHSV(itIsoDoseLevel->GetColor()[0], itIsoDoseLevel->GetColor()[1], itIsoDoseLevel->GetColor()[2], &hsv[0], &hsv[1], &hsv[2]);
          transferFunction->AddHSVPoint(itIsoDoseLevel->GetDoseValue()*referenceDose, hsv[0], hsv[1], hsv[2], 1.0, 1.0);
        }
      }

      mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
      mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
      mitkTransFunc->SetColorTransferFunction(transferFunction);
      mitkTransFuncProp->SetValue(mitkTransFunc);
      node->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);


      renderingModeProp->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);

    node->SetProperty("Image Rendering.Mode", renderingModeProp);
    node->SetProperty("opacity", mitk::FloatProperty::New(0.5));
  }
};
