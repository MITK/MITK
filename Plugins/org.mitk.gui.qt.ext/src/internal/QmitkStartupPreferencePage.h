/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStartupPreferencePage_h
#define QmitkStartupPreferencePage_h

#include <berryIQtPreferencePage.h>

namespace Ui
{
  class QmitkStartupPreferencePage;
}

class QmitkStartupPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkStartupPreferencePage();
  ~QmitkStartupPreferencePage() override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  void Init(berry::IWorkbench::Pointer) override;
  void PerformCancel() override;
  bool PerformOk() override;
  void Update() override;

private:
  Ui::QmitkStartupPreferencePage* m_Ui;
  QWidget* m_Control;
};

#endif
