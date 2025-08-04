/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonSettingsView_h
#define QmitkPythonSettingsView_h

#include <QmitkAbstractView.h>

namespace Ui
{
  class QmitkPythonSettingsView;
}

class QmitkPythonSettingsView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkPythonSettingsView(QObject* parent = nullptr);
  ~QmitkPythonSettingsView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private:
  void DeleteSelectedVenvs();
  void RefreshVenvsTreeWidget();
  void PopulateVenvsTreeWidget();
  void CalculateAllVenvSizes();

  Ui::QmitkPythonSettingsView* m_Ui;
};

#endif
