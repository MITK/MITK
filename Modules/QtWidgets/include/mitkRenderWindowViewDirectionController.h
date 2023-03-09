/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindowViewDirectionController_h
#define mitkRenderWindowViewDirectionController_h

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "mitkRenderWindowLayerUtilities.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /**
  * The RenderWindowViewDirectionController is used to manipulate the 'sliceNavigationController' of a given base renderer.
  * The 'sliceNavigationController' is used to set the view direction / camera perspective of a base renderer.
  * The view direction can be changed to 'mitk::AnatomicalPlane::Axial', 'mitk::AnatomicalPlane::Coronal'
  * or 'mitk::AnatomicalPlane::Sagittal'.
  *
  * Functions with 'mitk::BaseRenderer* renderer' have 'nullptr' as their default argument.
  * Using the nullptr these functions operate on all base renderer.
  * Giving a specific base renderer will change the view direction only for the given renderer.
  */
  class MITKQTWIDGETS_EXPORT RenderWindowViewDirectionController
  {
  public:

    RenderWindowViewDirectionController();
    /**
    * @brief Set the data storage on which to work.
    */
    void SetDataStorage(DataStorage::Pointer dataStorage);
    /**
    * @brief Set the controlled base renderer.
    */
    void SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer);

    // wrapper functions to change the view direction
    /**
    * @brief Set the ciew direction for the given renderer (nullptr = all renderer)
    * @param viewDirection  The view direction that should be used for this renderer as a string.
    *                       Currently "axial", "coronal" and "sagittal" is supported.
    * @param renderer       Pointer to the renderer instance for which the view direction should be changed.
    *                       If it is a nullptr (default) all controlled renderer will be affected.
    */
    void SetViewDirectionOfRenderer(const std::string& viewDirection, BaseRenderer* renderer = nullptr);
    /**
    * @brief Set the ciew direction for the given renderer (nullptr = all renderer)
    * @param viewDirection  The view direction that should be used for this renderer.
    * @param renderer       Pointer to the renderer instance for which the view direction should be changed.
    *                       If it is a nullptr (default) nothing happens.
    */
    void SetViewDirectionOfRenderer(AnatomicalPlane viewDirection, BaseRenderer* renderer = nullptr);
    /**
    * @brief Reinitialize the given renderer with the currently visible nodes.
    * @param renderer       Pointer to the renderer instance which should be reinitialized.
    *                       If it is a nullptr (default) all controlled renderer will be affected.
    */
    void InitializeViewByBoundingObjects(const BaseRenderer* renderer);

  private:

    DataStorage::Pointer m_DataStorage;
    RenderWindowLayerUtilities::RendererVector m_ControlledRenderer;
  };

} // namespace mitk

#endif
