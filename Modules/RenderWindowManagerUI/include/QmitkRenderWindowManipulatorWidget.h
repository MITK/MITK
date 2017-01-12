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

#ifndef QMITKRENDERWINDOWMANIPULATORWIDGET_H
#define QMITKRENDERWINDOWMANIPULATORWIDGET_H

// render window manager UI module
#include "MitkRenderWindowManagerUIExports.h"
#include "ui_QmitkRenderWindowManipulatorWidget.h"

// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowViewDirectionController.h>
#include <QmitkRenderWindowDataModel.h>

// qt
#include <QWidget>

/**
* The QmitkRenderWindowManipulatorWidget offers a GUI to manipulate the base renderer / render windows of the MITK workbench.
* The widgets supports adding a layer to an active render window, moving layers up and down, removing layers,
* resetting layers (hiding them) or removing all layers at once.
*
* In order to use this widget, a (e.g.) plugin has to set the controlled renderer, which will be forwarded to
* a render window layer controller and a render window view direction controller.
* The plugin also has to provide a Q_SLOT that is connected to the 'AddLayerButtonClicked'-Q_SIGNAL of this widget.
* This allows for a customized add-layer functionality.
*/
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkRenderWindowManipulatorWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkRenderWindowManipulatorWidget(mitk::DataStorage::Pointer dataStorage, QWidget* parent = nullptr);
  /**
  * @brief Set the controlled base renderer.
  */
  void SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer);
  /**
  * @brief Set the currently selected render window
  *
  * @param renderWindowId   the text inside the combo box
  */
  void SetActiveRenderWindow(const QString &renderWindowId);
  /**
  * @brief 
  *
  * @param dataNode   k
  */
  void AddLayer(mitk::DataNode* dataNode);
  /**
  * @brief
  *
  * @param dataNode   k
  */
  void HideDataNodeInAllRenderer(const mitk::DataNode* dataNode);

protected:

  void SetUpConnections();

Q_SIGNALS:
  void AddLayerButtonClicked();

private Q_SLOTS:

  void RemoveLayer();
  void SetAsBaseLayer();

  void MoveLayer(const QString &direction);

  void ResetRenderer();
  void ClearRenderer();

  void ChangeViewDirection(const QString &viewDirection);

private:

  void Init();

  Ui::QmitkRenderWindowManipulatorWidget m_Controls;

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkRenderWindowDataModel> m_RenderWindowDataModel;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
};

#endif // QMITKRENDERWINDOWMANIPULATORWIDGET_H
