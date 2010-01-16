/*=========================================================================
 
 Program:   BlueBerry Platform
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

public:

  void Init(IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* parent);
  QWidget* GetQtControl() const;

  bool PerformOk();
  void PerformCancel();
  void Update();

protected:

  void AddPath(const QString& path, bool updateCombo);
  void FillStyleCombo(const berry::IQtStyleManager::Style& currentStyle);

protected slots:

  void StyleChanged(int index);
  void AddPathClicked(bool checked);
  void RemovePathClicked(bool checked);
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
