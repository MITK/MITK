/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef QMITKAPPINSTANCESPREFERENCEPAGE_H_
#define QMITKAPPINSTANCESPREFERENCEPAGE_H_

#include <berryIQtPreferencePage.h>
#include <berryIPreferences.h>

#include <ui_QmitkAppInstancesPreferencePage.h>


class QmitkAppInstancesPreferencePage : public QObject, public berry::IQtPreferencePage
{

  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkAppInstancesPreferencePage();
  QmitkAppInstancesPreferencePage(const QmitkAppInstancesPreferencePage& other);

  void Init(berry::IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* parent);
  QWidget* GetQtControl() const;

  bool PerformOk();
  void PerformCancel();
  void Update();

private:

  Ui::QmitkAppInstancesPreferencePage controls;

  berry::IPreferences::Pointer prefs;

  QWidget* mainWidget;
};

#endif /* QMITKAPPINSTANCESPREFERENCEPAGE_H_ */
