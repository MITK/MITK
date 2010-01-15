/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYQTSTYLEPREFERENCEPAGE_H_
#define CHERRYQTSTYLEPREFERENCEPAGE_H_

#include <cherryIQtPreferencePage.h>

#include <cherryIPreferences.h>
#include <cherryIQtStyleManager.h>

#include <ui_cherryQtStylePreferencePage.h>

#include <QStringList>

namespace cherry {

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
  void FillStyleCombo(const cherry::IQtStyleManager::Style& currentStyle);

protected slots:

  void StyleChanged(int index);
  void AddPathClicked(bool checked);
  void RemovePathClicked(bool checked);
  void EditPathClicked(bool checked);

  void UpdatePathListButtons();

private:

  cherry::IQtStyleManager::Pointer styleManager;

  Ui::QtStylePreferencePageUI controls;
  cherry::IPreferences::Pointer m_StylePref;
  cherry::IQtStyleManager::Style oldStyle;

  cherry::IQtStyleManager::StyleList styles;

  QWidget* mainWidget;
};

}

#endif /* CHERRYQTSTYLEPREFERENCEPAGE_H_ */
