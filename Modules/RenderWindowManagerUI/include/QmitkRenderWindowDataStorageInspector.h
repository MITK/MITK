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

#ifndef QMITKRENDERWINDOWDATASTORAGEINSPECTOR_H
#define QMITKRENDERWINDOWDATASTORAGEWINSPECTOR_H

// render window manager UI module
#include "MitkRenderWindowManagerUIExports.h"
#include "ui_QmitkRenderWindowDataStorageInspector.h"

// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowViewDirectionController.h>
#include <QmitkRenderWindowDataStorageListModel.h>

// qt widgets module
#include <QmitkAbstractDataStorageInspector.h>

/**
* The 'QmitkRenderWindowDataStorageInspector' offers a GUI to manipulate the base renderer / render windows of the MITK workbench.
*
* In order to use this widget, a (e.g.) plugin has to set the controlled renderer, which will be forwarded to
* a render window view direction controller.
*/
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkRenderWindowDataStorageInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:

  QmitkRenderWindowDataStorageInspector(QWidget* parent = nullptr);

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

  Ui::QmitkRenderWindowDataStorageInspector m_Controls;

  std::unique_ptr<QmitkRenderWindowDataStorageListModel> m_StorageModel;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
};

#endif // QMITKRENDERWINDOWDATASTORAGEINSPECTOR_H
