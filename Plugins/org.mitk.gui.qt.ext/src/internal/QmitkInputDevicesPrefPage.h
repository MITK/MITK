/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITK_INPUTDEVICESPREFPAGE_H_
#define QMITK_INPUTDEVICESPREFPAGE_H_

#include "berryIQtPreferencePage.h"
#include <berryIPreferences.h>

#include <QHash>
#include <QGroupBox>
#include <QRadioButton>

class QWidget;
class QCheckBox;

namespace mitk {
struct IInputDeviceRegistry;
}

class QmitkInputDevicesPrefPage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  /**
  * Default constructor
  */
  QmitkInputDevicesPrefPage();

  /**
  * @see berry::IPreferencePage::Init(berry::IWorkbench::Pointer workbench)
  */
  void Init(berry::IWorkbench::Pointer workbench) override;

  /**
  * @see berry::IPreferencePage::CreateQtControl(void* parent)
  */
  void CreateQtControl(QWidget* widget) override;

  /**
  * @see berry::IPreferencePage::CreateQtControl()
  */
  QWidget* GetQtControl() const override;

  /**
  * @see berry::IPreferencePage::PerformOk()
  */
  bool PerformOk() override;

  /**
  * @see berry::IPreferencePage::PerformCancel()
  */
  void PerformCancel() override;

   /**
  * @see berry::IPreferencePage::Update()
  */
  void Update() override;

protected:

  QWidget* m_MainControl;

  QHash<QCheckBox*,QString> m_InputDevices;

  // specific for Wiimote
  QGroupBox* m_WiiMoteModes;
  QRadioButton* m_WiiMoteHeadTracking;
  QRadioButton* m_WiiMoteSurfaceInteraction;

  berry::IPreferences::Pointer m_InputDevicesPrefNode;

  mitk::IInputDeviceRegistry* GetInputDeviceRegistry() const;

};

#endif // QMITK_INPUTDEVICESPREFPAGE_H_
