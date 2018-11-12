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


#ifndef QMITKDATAMANAGERPREFERENCEPAGE_H_
#define QMITKDATAMANAGERPREFERENCEPAGE_H_

#include "berryIQtPreferencePage.h"
#include <org_mitk_gui_qt_datamanager_Export.h>
#include <berryIPreferences.h>

class QWidget;
class QCheckBox;

struct MITK_QT_DATAMANAGER QmitkDataManagerPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkDataManagerPreferencePage();

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* widget) override;

  QWidget* GetQtControl() const override;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  bool PerformOk() override;

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  void PerformCancel() override;

  ///
  /// \see IPreferencePage::Update()
  ///
  void Update() override;

protected:
  QWidget* m_MainControl;
  QCheckBox* m_EnableSingleEditing;
  QCheckBox* m_PlaceNewNodesOnTop;
  QCheckBox* m_ShowHelperObjects;
  QCheckBox* m_ShowNodesContainingNoData;
  QCheckBox* m_UseSurfaceDecimation;
  QCheckBox* m_AllowParentChange;
  berry::IPreferences::Pointer m_DataManagerPreferencesNode;
};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */
