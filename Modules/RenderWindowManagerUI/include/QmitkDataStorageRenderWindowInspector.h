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

#ifndef QMITKDATASTORAGERENDERWINDOWINSPECTOR_H
#define QMITKDATASTORAGERENDERWINDOWINSPECTOR_H

// render window manager UI module
#include "MitkRenderWindowManagerUIExports.h"
#include "ui_QmitkDataStorageRenderWindowInspector.h"

// render window manager module
#include <mitkRenderWindowViewDirectionController.h>
#include <QmitkDataStorageRenderWindowListModel.h>

// qt widgets module
#include <QmitkAbstractDataStorageInspector.h>

/**
* The 'QmitkDataStorageRenderWindowInspector' offers a GUI to manipulate the base renderer / render windows of the MITK workbench.
* The widgets supports adding a layer to an active render window, moving layers up and down, removing layers,
* resetting layers (hiding them) or removing all layers at once.
*
* In order to use this widget, a (e.g.) plugin has to set the controlled renderer, which will be forwarded to
* a render window view direction controller.
*/
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkDataStorageRenderWindowInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:

  QmitkDataStorageRenderWindowInspector(QWidget* parent = nullptr);

  // override from 'QmitkAbstractDataStorageInspector'
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  virtual QAbstractItemView* GetView() override;
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  virtual const QAbstractItemView* GetView() const override;
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  virtual void SetSelectionMode(SelectionMode mode) override;
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  virtual SelectionMode GetSelectionMode() const override;
  /**
  * @brief Set the controlled base renderer.
  */
  void SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer);
  /**
  * @brief Set the currently selected render window.
  *
  * @param renderWindowId   the text inside the combo box
  */
  void SetActiveRenderWindow(const QString& renderWindowId);

private Q_SLOTS:

  void SetAsBaseLayer();

  void ResetRenderer();

  void ChangeViewDirection(const QString& viewDirection);

private:

  virtual void Initialize() override;
  void SetUpConnections();

  Ui::QmitkDataStorageRenderWindowInspector m_Controls;

  std::unique_ptr<QmitkDataStorageRenderWindowListModel> m_StorageModel;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
};

#endif // QMITKDATASTORAGERENDERWINDOWINSPECTOR_H
