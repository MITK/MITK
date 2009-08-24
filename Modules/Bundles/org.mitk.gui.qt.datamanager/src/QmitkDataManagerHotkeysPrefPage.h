/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-02-10 14:14:32 +0100 (Di, 10 Feb 2009) $
 Version:   $Revision: 16224 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKDATAMANAGERHOTKEYSPREFPAGE_H_
#define QMITKDATAMANAGERHOTKEYSPREFPAGE_H_

#include <QWidget>
#include "cherryIQtPreferencePage.h"
#include "mitkQtDataManagerDll.h"
#include <map>
class QmitkHotkeyLineEdit;

struct MITK_QT_DATAMANAGER QmitkDataManagerHotkeysPrefPage : public cherry::IQtPreferencePage
{
  Q_OBJECT

public:
  QmitkDataManagerHotkeysPrefPage(QWidget* parent = 0, Qt::WindowFlags f = 0);

  ///
  /// Invoked when the OK button was clicked in the preferences dialog
  ///
  virtual bool PerformOk();

  ///
  /// Invoked when the Cancel button was clicked in the preferences dialog
  ///
  virtual void PerformCancel();
protected:
  ///
  /// The node from which the properties are taken (will be catched from the preferences service in ctor)
  ///
  cherry::IPreferences::WeakPtr m_DataManagerHotkeysPreferencesNode;

  ///
  /// Maps a label to hotkey lineedit, e.g. "Toggle Visibility of selected nodes" => QmitkHotkeyLineEdit
  ///
  std::map<QString, QmitkHotkeyLineEdit*> m_HotkeyEditors;
};

#endif /* QMITKDATAMANAGERHOTKEYSPREFPAGE_H_ */
