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

// org mitk gui common plugin
#include <mitkIRenderWindowPartListener.h>

// org mitk gui qt common plugin
#include <QmitkAbstractView.h>

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

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

private Q_SLOT:

  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  void OnAddPointSetClicked();

private:

  void CreateQtPartControl(QWidget *parent) override;

  Ui::QmitkPointSetInteractionViewControls* m_Controls;

};

#endif // QMITKPOINTSETINTERACTIONVIEW_H
