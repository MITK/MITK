/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRemeshingView_h
#define QmitkRemeshingView_h

#include <QmitkAbstractView.h>
#include <QmitkSingleNodeSelectionWidget.h>

namespace Ui
{
  class QmitkRemeshingViewControls;
}

class QmitkRemeshingView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkRemeshingView();
  ~QmitkRemeshingView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private slots:
  void OnSurfaceChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes);
  void OnVertexCountChanged(int vertexCount);
  void OnCalculateNormalsChanged(int checkState);
  void OnDecimateButtonClicked();

private:
  void EnableWidgets(bool enable);

  Ui::QmitkRemeshingViewControls* m_Controls;
};

#endif
