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

#include "cherryIQtPreferencePage.h"
#include "mitkQtDataManagerDll.h"

#include <map>
#include <QWidget>

class QmitkHotkeyLineEdit;

struct MITK_QT_DATAMANAGER QmitkDataManagerHotkeysPrefPage : public cherry::IQtPreferencePage
{

public:
  QmitkDataManagerHotkeysPrefPage();

  void Init(cherry::IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* parent);

  QWidget* GetQtControl() const;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  virtual bool PerformOk();

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  virtual void PerformCancel();

  ///
  /// \see IPreferencePage::Update()
  ///
  virtual void Update();
protected:
  ///
  /// The node from which the properties are taken (will be catched from the preferences service in ctor)
  ///
  cherry::IPreferences::WeakPtr m_DataManagerHotkeysPreferencesNode;

  ///
  /// Maps a label to hotkey lineedit, e.g. "Toggle Visibility of selected nodes" => QmitkHotkeyLineEdit
  ///
  std::map<QString, QmitkHotkeyLineEdit*> m_HotkeyEditors;

  QWidget* m_MainControl;
};

#endif /* QMITKDATAMANAGERHOTKEYSPREFPAGE_H_ */
