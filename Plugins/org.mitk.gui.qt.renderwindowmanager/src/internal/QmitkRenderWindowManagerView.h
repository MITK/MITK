/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowManagerView_h
#define QmitkRenderWindowManagerView_h

// render window manager UI module
#include <QmitkRenderWindowDataStorageInspector.h>

// mitk gui qt application
#include <QmitkDataNodeContextMenu.h>

// mitk gui common plugin
#include <mitkIRenderWindowPartListener.h>

// mitk gui qt common plugin
#include <QmitkAbstractView.h>

#include <memory>

namespace Ui
{
  class QmitkRenderWindowManagerControls;
}

/**
* @brief RenderWindowManager
*/
class QmitkRenderWindowManagerView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkRenderWindowManagerView();
  ~QmitkRenderWindowManagerView() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

protected:

  void SetFocus() override {}

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
  std::unique_ptr<Ui::QmitkRenderWindowManagerControls> m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart = nullptr;

  QmitkRenderWindowDataStorageInspector* m_RenderWindowInspector;
  QAbstractItemView* m_InspectorView;
  QmitkDataNodeContextMenu* m_DataNodeContextMenu;

  QItemSelectionModel* GetDataNodeSelectionModel() const override;
};

#endif
