/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKCONFIGPREFERENCEPAGE_H
#define QMITKCONFIGPREFERENCEPAGE_H

#include <unordered_set>
#include <string>
#include <memory>

#include <berryIQtPreferencePage.h>

namespace Ui {
class QmitkConfigPreferencePage;
}

class QmitkConfigPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkConfigPreferencePage();
  ~QmitkConfigPreferencePage();

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:

  Q_SLOT void DeleteSelectedParam();

  std::unique_ptr<Ui::QmitkConfigPreferencePage> ui;
  QWidget* pageWidget;

  berry::IWorkbench* workbench;

  std::unordered_set<std::string> m_deletedParams;
};

#endif // QMITKCONFIGPREFERENCEPAGE_H
