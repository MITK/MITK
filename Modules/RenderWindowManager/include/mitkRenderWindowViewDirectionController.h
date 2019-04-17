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

#ifndef MITKRENDERWINDOWVIEWDIRECTIONCONTROLLER_H
#define MITKRENDERWINDOWVIEWDIRECTIONCONTROLLER_H

// render window manager module
#include "MitkRenderWindowManagerExports.h"
#include "mitkRenderWindowLayerUtilities.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /**
  * The RenderWindowViewDirectionController is used to manipulate the 'sliceNavigationController' of a given base renderer.
  * The 'sliceNavigationController' is used to set the view direction / camera perspective of a base renderer.
  * The view direction can changed to 'mitk::SliceNavigationController::Axial', 'mitk::SliceNavigationController::Frontal'
  * or 'mitk::SliceNavigationController::Sagittal'.
  *
  * Functions with 'mitk::BaseRenderer* renderer' have 'nullptr' as their default argument. Using the nullptr
  * these functions operate on all base renderer. Giving a specific base renderer will modify the view direction only for the given renderer.
  */
  class MITKRENDERWINDOWMANAGER_EXPORT RenderWindowViewDirectionController
  {
  public:
    
    using ViewDirection = mitk::SliceNavigationController::ViewDirection;

    RenderWindowViewDirectionController();
    /**
    * @brief Set the data storage on which to work.
    */
    void SetDataStorage(DataStorage::Pointer dataStorage);
    /**
    * @brief Set the controlled base renderer.
    */
    void SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer);

    // wrapper functions to modify the view direction
    /**
    * @brief Set the view direction for the given renderer (nullptr = all renderer)
    * @param viewDirection  The view direction that should be used for this renderer as a string.
    *                       Currently "axial", "coronal" and "sagittal" is supported.
    * @param renderer       Pointer to the renderer instance for which the view direction should be changed.
    *                       If it is a nullptr (default) all controlled renderer will be affected.
    */
    void SetViewDirectionOfRenderer(const std::string& viewDirection, BaseRenderer* renderer = nullptr);
    /**
    * @brief Set the view direction for the given renderer (nullptr = all renderer)
    * @param viewDirection  The view direction that should be used for this renderer.
    * @param renderer       Pointer to the renderer instance for which the view direction should be changed.
    *                       If it is a nullptr (default) nothing happens.
    */
    void SetViewDirectionOfRenderer(ViewDirection viewDirection, BaseRenderer* renderer = nullptr);
    /**
    * @brief Reinitialize the given renderer with the currently visible nodes.
    * @param renderer       Pointer to the renderer instance for which the view direction should be changed.
    *                       If it is a nullptr (default) all controlled renderer will be affected.
    */
    void InitializeViewByBoundingObjects(const BaseRenderer* renderer);

  private:

    DataStorage::Pointer m_DataStorage;
    RenderWindowLayerUtilities::RendererVector m_ControlledRenderer;
  };

} // namespace mitk

#endif // MITKRENDERWINDOWVIEWDIRECTIONCONTROLLER_H
