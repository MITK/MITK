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

// render window manager
#include "MitkRenderWindowManagerExports.h"

// mitk
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /*
  */
  class MITKRENDERWINDOWMANAGER_EXPORT RenderWindowViewDirectionController
  {
  public:
    
    typedef std::vector<BaseRenderer::Pointer> RendererVector;

    RenderWindowViewDirectionController();
    virtual ~RenderWindowViewDirectionController();
    /**
    * @brief set the data storage on which to work
    */
    void SetDataStorage(DataStorage::Pointer dataStorage);
    DataStorage::Pointer GetDataStorage() { return m_DataStorage; };
    /**
    * @brief set the controlled base renderer by specifying the corresponding render windows
    */
    void SetControlledRenderer(const RenderingManager::RenderWindowVector &renderWindows);
    /**
    * @brief set the controlled base renderer
    */
    void SetControlledRenderer(RendererVector controlledRenderer);
    RendererVector GetControlledRenderer() { return m_ControlledRenderer; };

    // wrapper functions to modify the view direction
    /**
    * @brief set the view direction for the given renderer (nullptr = all renderer)
    */
    void SetViewDirectionOfRenderer(const std::string &viewDirection, BaseRenderer* renderer = nullptr);

  private:

    void InitializeViewByBoundingObjects(const BaseRenderer* renderer);

    DataStorage::Pointer m_DataStorage;
    RendererVector m_ControlledRenderer;
  };

} // namespace mitk

#endif // MITKRENDERWINDOWVIEWDIRECTIONCONTROLLER_H
