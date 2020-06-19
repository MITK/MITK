/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKPOINTSETINTERACTIONVIEW_H
#define QMITKPOINTSETINTERACTIONVIEW_H

#include "ui_QmitkPointSetInteractionViewControls.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkWeakPointer.h>

// org mitk gui common plugin
#include <mitkILifecycleAwarePart.h>
#include <mitkIRenderWindowPartListener.h>

// org mitk gui qt common plugin
#include <QmitkAbstractView.h>

#include <berryISelectionListener.h>

/**
* @brief
*
*
*/
class QmitkPointSetInteractionView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkPointSetInteractionView();
  ~QmitkPointSetInteractionView() override;

  void SetFocus() override;

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;
  void NodeChanged(const mitk::DataNode* node) override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

protected Q_SLOT:

  void OnAddPointSetClicked();

private:

  void CreateQtPartControl(QWidget *parent) override;

  Ui::QmitkPointSetInteractionViewControls* m_Controls;

  mitk::WeakPointer<mitk::DataNode> m_SelectedPointSetNode;

};

#endif // QMITKPOINTSETINTERACTIONVIEW_H
