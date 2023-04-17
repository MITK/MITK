/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationTaskListView_h
#define QmitkSegmentationTaskListView_h

#include <berryIWorkbenchListener.h>
#include <QmitkAbstractView.h>

namespace Ui
{
  class QmitkSegmentationTaskListView;
}

class QmitkSegmentationTaskListView : public QmitkAbstractView, public berry::IWorkbenchListener
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkSegmentationTaskListView();
  ~QmitkSegmentationTaskListView() override;

private:
  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;
  void NodeAdded(const mitk::DataNode*) override;
  void NodeRemoved(const mitk::DataNode* removedNode) override;

  bool PreShutdown(berry::IWorkbench*, bool) override;

  Ui::QmitkSegmentationTaskListView* m_Ui;
};

#endif
