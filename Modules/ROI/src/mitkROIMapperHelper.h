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
    /** \brief Apply %ROI properties at a certain time step to the given actor.
      */
    void ApplyIndividualProperties(const ROI::Element& roi, TimeStepType t, vtkActor* actor);

    /** \brief Create an actor for the %ROI caption located at a certain attachment point considering several properties. 
      */
    vtkSmartPointer<vtkCaptionActor2D> CreateCaptionActor(const std::string& caption, const Point3D& attachmentPoint, vtkProperty* property, const DataNode* dataNode, const BaseRenderer* renderer);

    /** \brief Substitute all placeholders in a caption with corresponding property values.
      *
      * \sa ROI
      */
    std::string ParseCaption(const std::string& captionTemplate, const ROI::Element& roi, TimeStepType t = 0);

    /** \brief Set common default properties for both 2-d and 3-d %ROI mappers.
      */
    void SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override);

    /** \brief Syntactic sugar for getting %ROI properties.
      */
    template <class T>
    const T* GetConstProperty(const std::string& propertyKey, const ROI::Element& roi, TimeStepType t)
    {
      auto property = roi.GetConstProperty(propertyKey, t);

      if (property.IsNotNull())
        return dynamic_cast<const T*>(property.GetPointer());

      return nullptr;
    }
  }
}

#endif
