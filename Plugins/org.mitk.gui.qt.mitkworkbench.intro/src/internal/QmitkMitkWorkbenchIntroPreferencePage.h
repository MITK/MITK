/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMitkWorkbenchIntroPreferencePage_h
#define QmitkMitkWorkbenchIntroPreferencePage_h

#include <berryIQtPreferencePage.h>

namespace Ui
{
  class QmitkMitkWorkbenchIntroPreferencePage;
}

class QmitkMitkWorkbenchIntroPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkMitkWorkbenchIntroPreferencePage();
  ~QmitkMitkWorkbenchIntroPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:
  Ui::QmitkMitkWorkbenchIntroPreferencePage* m_Ui;
  QWidget* m_Control;
};

#endif
