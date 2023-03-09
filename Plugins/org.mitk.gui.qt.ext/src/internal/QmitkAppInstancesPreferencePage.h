/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkAppInstancesPreferencePage_h
#define QmitkAppInstancesPreferencePage_h

#include <berryIQtPreferencePage.h>

#include <ui_QmitkAppInstancesPreferencePage.h>

class QmitkAppInstancesPreferencePage : public QObject, public berry::IQtPreferencePage
{

  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkAppInstancesPreferencePage();
  QmitkAppInstancesPreferencePage(const QmitkAppInstancesPreferencePage& other);

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:

  Ui::QmitkAppInstancesPreferencePage controls;
  QWidget* mainWidget;
};

#endif
