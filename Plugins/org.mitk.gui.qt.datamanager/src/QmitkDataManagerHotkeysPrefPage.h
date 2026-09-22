/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataManagerHotkeysPrefPage_h
#define QmitkDataManagerHotkeysPrefPage_h

#include <org_mitk_gui_qt_datamanager_Export.h>

// blueberry ui qt plugin
#include <berryIQtPreferencePage.h>

// qt
#include <QWidget>

// c++
#include <vector>

class QmitkHotkeyLineEdit;

struct MITK_QT_DATAMANAGER QmitkDataManagerHotkeysPrefPage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkDataManagerHotkeysPrefPage();

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;

  QWidget* GetQtControl() const override;

  /**
  * @brief \see IPreferencePage::PerformOk()
  */
  bool PerformOk() override;
  /**
  * @brief \see IPreferencePage::PerformCancel()
  */
  void PerformCancel() override;
  /**
  * @brief \see IPreferencePage::Update()
  */
  void Update() override;

protected:
  /**
  * @brief A single hotkey row.
  *
  * The preference key is deliberately separate from the displayed label so
  * that labels can be reworded without discarding hotkeys that users have
  * customized. QmitkNodeTableViewKeyFilter looks up the same keys.
  */
  struct Hotkey
  {
    QString key;
    QString label;
    QmitkHotkeyLineEdit* editor;
  };

  std::vector<Hotkey> m_Hotkeys;

  QWidget* m_MainControl;
};

#endif
