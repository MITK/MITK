/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
