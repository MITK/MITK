/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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


void mitk::ConfigureNodeAsDoseNode(mitk::DataNode* doseNode, const mitk::IsoDoseLevelSet* colorPreset, mitk::DoseValueAbs referenceDose, bool showColorWashGlobal)
{
  if (!doseNode) return;

  mitk::Image::Pointer doseImage = dynamic_cast<mitk::Image*>(doseNode->GetData());

  if (doseImage != nullptr)
  {
    //set some specific color wash and isoline properties
    doseNode->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), showColorWashGlobal);

    //Set reference dose property
    doseNode->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);

    mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(dynamic_cast<mitk::IsoDoseLevelSet*>(colorPreset->Clone().GetPointer()));

    doseNode->SetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(), levelSetProp);

    mitk::RenderingModeProperty::Pointer renderingModeProp = mitk::RenderingModeProperty::New();

    if (showColorWashGlobal)
    {
      //Generating the color wash
      vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

      for (mitk::IsoDoseLevelSet::ConstIterator itIsoDoseLevel = colorPreset->Begin(); itIsoDoseLevel != colorPreset->End(); ++itIsoDoseLevel)
      {
        float *hsv = new float[3];
        //used for transfer rgb to hsv
        vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();
        if (itIsoDoseLevel->GetVisibleColorWash()){
          cCalc->RGBToHSV(itIsoDoseLevel->GetColor()[0], itIsoDoseLevel->GetColor()[1], itIsoDoseLevel->GetColor()[2], &hsv[0], &hsv[1], &hsv[2]);
          transferFunction->AddHSVPoint(itIsoDoseLevel->GetDoseValue()*referenceDose, hsv[0], hsv[1], hsv[2], 1.0, 1.0);
        }
      }

      mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
      mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
      mitkTransFunc->SetColorTransferFunction(transferFunction);
      mitkTransFuncProp->SetValue(mitkTransFunc);

      doseNode->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);
      renderingModeProp->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);
    }
    else
    {
      //Set rendering mode to levelwindow color mode
      renderingModeProp->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
    }

    doseNode->SetProperty("Image Rendering.Mode", renderingModeProp);
    doseNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
  }
};


void mitk::ConfigureNodeAsIsoLineNode(mitk::DataNode* doseOutlineNode, const mitk::IsoDoseLevelSet* colorPreset, mitk::DoseValueAbs referenceDose, bool showIsolinesGlobal)
{
  if (doseOutlineNode != nullptr)
  {
    mitk::Image::Pointer doseImage = dynamic_cast<mitk::Image*>(doseOutlineNode->GetData());

    if (doseImage != nullptr)
    {
      mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(dynamic_cast<mitk::IsoDoseLevelSet*>(colorPreset->Clone().GetPointer()));

      mitk::IsoDoseLevelVector::Pointer levelVector = mitk::IsoDoseLevelVector::New();
      mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(levelVector);

      doseOutlineNode->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), showIsolinesGlobal);
      doseOutlineNode->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);

      doseOutlineNode->SetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(), levelSetProp);

      doseOutlineNode->SetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(), levelVecProp);

      //set the outline properties
      doseOutlineNode->SetBoolProperty("outline binary", true);
      doseOutlineNode->SetProperty("helper object", mitk::BoolProperty::New(true));
      doseOutlineNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));

      //set the dose mapper for outline drawing; the colorwash is realized by the imagevtkmapper2D
      mitk::DoseImageVtkMapper2D::Pointer contourMapper = mitk::DoseImageVtkMapper2D::New();
      doseOutlineNode->SetMapper(1, contourMapper);
    }
  }
};
