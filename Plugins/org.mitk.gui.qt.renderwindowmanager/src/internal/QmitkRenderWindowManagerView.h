/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKRENDERWINDOWMANAGERVIEW_H
#define QMITKRENDERWINDOWMANAGERVIEW_H

// render window manager plugin
#include "ui_QmitkRenderWindowManagerControls.h"

// render window manager UI module
#include <QmitkRenderWindowDataStorageInspector.h>

// mitk gui qt application
#include <QmitkDataNodeContextMenu.h>

// mitk gui common plugin
#include <mitkIRenderWindowPartListener.h>

// mitk gui qt common plugin
#include <QmitkAbstractView.h>

/**
* @brief RenderWindowManager
*/
class QmitkRenderWindowManagerView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

protected:

  void SetFocus() override;

  void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  /**
  * @brief Called when the user changes the render window selection in the combo box.
  *
  * @param renderWindowId   The text inside the combo box.
  */
  void OnRenderWindowSelectionChanged(const QString& renderWindowId);

private:

  void SetControlledRenderer();

  void RenderWindowChanged();

  QWidget* m_Parent;
  Ui::QmitkRenderWindowManagerControls m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart;

  QmitkRenderWindowDataStorageInspector* m_RenderWindowInspector;
  QAbstractItemView* m_InspectorView;
  QmitkDataNodeContextMenu* m_DataNodeContextMenu;

  QItemSelectionModel* GetDataNodeSelectionModel() const override;
};

#endif // QMITKRENDERWINDOWMANAGERVIEW_H
