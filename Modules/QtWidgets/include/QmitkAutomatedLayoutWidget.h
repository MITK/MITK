/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAutomatedLayoutDialog_h
#define QmitkAutomatedLayoutDialog_h

#include <MitkQtWidgetsExports.h>

#include <mitkDataStorage.h>

#include <QWidget>

namespace Ui
{
  class QmitkAutomatedLayoutWidget;
}

class MITKQTWIDGETS_EXPORT QmitkAutomatedLayoutWidget : public QWidget
{
  Q_OBJECT

private Q_SLOTS:
  void OnSetLayoutClicked();
  void OnSelectionDialogClosed();

Q_SIGNALS:
  void SetDataBasedLayout(const QList<mitk::DataNode::Pointer>& nodes);


public:
  explicit QmitkAutomatedLayoutWidget(QWidget* parent = nullptr);
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

private:

  Ui::QmitkAutomatedLayoutWidget* m_Controls;
  mitk::DataStorage::Pointer m_DataStorage;
};


#endif
