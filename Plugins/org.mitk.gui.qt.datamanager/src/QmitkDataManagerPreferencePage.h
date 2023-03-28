/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkDataManagerPreferencePage_h
#define QmitkDataManagerPreferencePage_h

#include "berryIQtPreferencePage.h"
#include <org_mitk_gui_qt_datamanager_Export.h>

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
  QCheckBox* m_AllowParentChange;
};

#endif
