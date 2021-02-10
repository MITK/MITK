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

#include <mitkDataNode.h>
#include <mitkIRenderWindowPartListener.h>

#include <QmitkAbstractView.h>
#include <QmitkSingleNodeSelectionWidget.h>

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

  void OnCurrentSelectionChanged(QmitkSingleNodeSelectionWidget::NodeList nodes);
  void OnAddPointSetClicked();

private:

  void CreateQtPartControl(QWidget *parent) override;

  Ui::QmitkPointSetInteractionViewControls* m_Controls;

};

#endif
