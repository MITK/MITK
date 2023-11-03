/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIMapperHelper_h
#define mitkROIMapperHelper_h

#include <mitkBaseRenderer.h>
#include <mitkROI.h>

#include <vtkCaptionActor2D.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  namespace ROIMapperHelper
  {
    void ApplyIndividualProperties(const IPropertyProvider* properties, vtkActor* actor);

    vtkSmartPointer<vtkCaptionActor2D> CreateCaptionActor(const std::string& caption, const Point3D& attachmentPoint, vtkProperty* property, const DataNode* dataNode, const BaseRenderer* renderer);

    std::string ParseCaption(const std::string& captionTemplate, const ROI::Element& roi);

    void SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override);

    template <class T>
    const T* GetConstProperty(const std::string& propertyKey, const mitk::IPropertyProvider* properties)
    {
      auto property = properties->GetConstProperty(propertyKey);

      if (property.IsNotNull())
        return dynamic_cast<const T*>(property.GetPointer());

      return nullptr;
    }
  }
}

#endif
