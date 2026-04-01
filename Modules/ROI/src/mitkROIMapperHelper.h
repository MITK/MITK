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
  /**
   * \brief Helper functions shared by ROIMapper2D and ROIMapper3D.
   *
   * Provides utilities for applying ROI-specific visual properties to VTK actors,
   * creating caption actors for ROI labels, parsing caption template strings,
   * setting default properties, and retrieving typed ROI properties.
   *
   * \sa ROIMapper2D, ROIMapper3D, ROI
   */
  namespace ROIMapperHelper
  {
    /**
     * \brief Apply individual ROI properties at a certain time step to the given actor.
     *
     * Sets wireframe representation, disables lighting, and applies color, opacity,
     * and line width from the ROI element's properties at the specified time step.
     *
     * \param roi The ROI element whose properties are applied.
     * \param t The time step from which to read properties.
     * \param actor The VTK actor to configure.
     */
    void ApplyIndividualProperties(const ROI::Element& roi, TimeStepType t, vtkActor* actor);

    /**
     * \brief Create a VTK caption actor for an ROI label.
     *
     * The caption is positioned at the given attachment point and styled according
     * to the color and opacity of the provided VTK property, as well as font properties
     * (size, bold, italic) read from the data node.
     *
     * \param caption The text string to display.
     * \param attachmentPoint The 3D point where the caption is anchored.
     * \param property The VTK property from which to read color and opacity.
     * \param dataNode The data node from which to read font properties.
     * \param renderer The renderer for renderer-specific property lookup.
     * \return A new vtkCaptionActor2D configured with the specified text and style.
     */
    vtkSmartPointer<vtkCaptionActor2D> CreateCaptionActor(const std::string& caption, const Point3D& attachmentPoint, vtkProperty* property, const DataNode* dataNode, const BaseRenderer* renderer);

    /**
     * \brief Substitute all placeholders in a caption template with corresponding property values.
     *
     * Placeholders are delimited by curly braces (e.g., \c {ID}, \c {name}).
     * The special placeholder \c {ID} is replaced by the ROI element's numeric ID.
     * All other placeholders are looked up as properties of the ROI element at the
     * given time step. If no placeholders are found, the template string is returned as-is.
     *
     * \param captionTemplate The template string containing placeholders.
     * \param roi The ROI element from which to read property values.
     * \param t The time step for property lookup (default is 0).
     * \return The caption string with all placeholders substituted.
     *
     * \sa ROI
     */
    std::string ParseCaption(const std::string& captionTemplate, const ROI::Element& roi, TimeStepType t = 0);

    /**
     * \brief Set common default properties for both 2D and 3D ROI mappers.
     *
     * Configures default values for opacity, font style (bold, italic, size),
     * and caption template ("{ID}\\n{name}").
     *
     * \param node The data node to decorate with default properties.
     * \param renderer The renderer for renderer-specific properties, or nullptr for global defaults.
     * \param override Whether to overwrite existing properties.
     */
    void SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override);

    /**
     * \brief Retrieve a typed const property from an ROI element.
     *
     * Convenience template that retrieves a property by key from the ROI element
     * at the given time step and performs a dynamic_cast to the requested type.
     *
     * \tparam T The property type to cast to (e.g., ColorProperty, FloatProperty).
     * \param propertyKey The key of the property to look up.
     * \param roi The ROI element to query.
     * \param t The time step for property lookup.
     * \return Pointer to the property cast to type T, or nullptr if not found or cast fails.
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
