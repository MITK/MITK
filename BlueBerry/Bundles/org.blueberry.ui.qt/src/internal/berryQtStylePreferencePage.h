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


#ifndef BERRYQTSTYLEPREFERENCEPAGE_H_
#define BERRYQTSTYLEPREFERENCEPAGE_H_

#include <berryIQtPreferencePage.h>

#include <berryIPreferences.h>
#include <berryIQtStyleManager.h>

#include <ui_berryQtStylePreferencePage.h>

#include <QStringList>

namespace berry {

class QtStylePreferencePage : public QObject, public IQtPreferencePage
{

  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QtStylePreferencePage();
  QtStylePreferencePage(const QtStylePreferencePage& other);

  void Init(IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* parent);
  QWidget* GetQtControl() const;

  bool PerformOk();
  void PerformCancel();
  void Update();

protected:

  void AddPath(const QString& path, bool updateCombo);
  void FillStyleCombo(const berry::IQtStyleManager::Style& currentStyle);
  void FillIconThemeComboBox(const QString currentIconTheme);

protected slots:

  void StyleChanged(int /*index*/);
  void IconThemeChanged(int /*index*/);
  void AddPathClicked(bool /*checked*/);
  void RemovePathClicked(bool /*checked*/);
  void EditPathClicked(bool checked);

  void UpdatePathListButtons();

private:

  berry::IQtStyleManager::Pointer styleManager;

  Ui::QtStylePreferencePageUI controls;
  berry::IPreferences::Pointer m_StylePref;
  berry::IQtStyleManager::Style oldStyle;

  berry::IQtStyleManager::StyleList styles;

  QWidget* mainWidget;
};

}

#endif /* BERRYQTSTYLEPREFERENCEPAGE_H_ */
