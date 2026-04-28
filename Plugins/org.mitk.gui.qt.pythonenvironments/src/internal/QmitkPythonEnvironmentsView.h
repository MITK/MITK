/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonEnvironmentsView_h
#define QmitkPythonEnvironmentsView_h

#include <QmitkAbstractView.h>
#include <memory>

namespace Ui
{
  class QmitkPythonEnvironmentsView;
}

class QmitkPythonEnvironmentsView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkPythonEnvironmentsView(QObject* parent = nullptr);
  ~QmitkPythonEnvironmentsView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private:
  void DeleteSelectedVenvs();
  void RefreshVenvsTreeWidget();
  void PopulateVenvsTreeWidget();
  void CalculateAllVenvSizes();

  std::unique_ptr<Ui::QmitkPythonEnvironmentsView> m_Ui;
};

#endif
