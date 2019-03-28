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

  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  virtual void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

protected:

  virtual void SetFocus() override;

  virtual void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  /**
  * @brief Called when the user changes the render window selection in the combo box.
  *
  * @param renderWindowId   The text inside the combo box.
  */
  void OnRenderWindowSelectionChanged(const QString& renderWindowId);

private:

  void SetControlledRenderer();

  QWidget* m_Parent;
  Ui::QmitkRenderWindowManagerControls m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart;

  QmitkRenderWindowDataStorageInspector* m_RenderWindowInspector;
  QAbstractItemView* m_InspectorView;
  QmitkDataNodeContextMenu* m_DataNodeContextMenu;

  virtual QItemSelectionModel* GetDataNodeSelectionModel() const override;
};

#endif // QMITKRENDERWINDOWMANAGERVIEW_H
