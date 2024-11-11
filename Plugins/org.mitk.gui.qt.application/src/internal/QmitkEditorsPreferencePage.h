/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkEditorsPreferencePage_h
#define QmitkEditorsPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <QScopedPointer>

namespace Ui
{
  class QmitkEditorsPreferencePage;
}

class QmitkEditorsPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkEditorsPreferencePage();
  ~QmitkEditorsPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget* widget) override;
  QWidget* GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:
  QWidget* m_Control;
  QScopedPointer<Ui::QmitkEditorsPreferencePage> m_Ui;
};

#endif
