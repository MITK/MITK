/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTSTYLEPREFERENCEPAGE_H_
#define BERRYQTSTYLEPREFERENCEPAGE_H_

#include <berryIQtPreferencePage.h>
#include <berryIQtStyleManager.h>
#include <berryIPreferences.h>

#include <ui_berryQtStylePreferencePage.h>

#include <QStringList>

namespace berry {

class QtStylePreferencePage : public QObject, public IQtPreferencePage
{

  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QtStylePreferencePage();

  void Init(IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

protected:

  void AddPath(const QString& path, bool updateCombo);
  void FillStyleCombo(const berry::IQtStyleManager::Style& currentStyle);
  void FillFontCombo(const QString& currentFont);

protected slots:

  void StyleChanged(int /*index*/);
  void FontChanged(int /*index*/);
  void AddPathClicked(bool /*checked*/);
  void RemovePathClicked(bool /*checked*/);
  void EditPathClicked(bool checked);

  void UpdatePathListButtons();

private:

  berry::IQtStyleManager* styleManager;

  Ui::QtStylePreferencePageUI controls;
  berry::IPreferences::Pointer m_StylePref;
  berry::IQtStyleManager::Style oldStyle;

  berry::IQtStyleManager::StyleList styles;

  QWidget* mainWidget;
};

}

#endif /* BERRYQTSTYLEPREFERENCEPAGE_H_ */
