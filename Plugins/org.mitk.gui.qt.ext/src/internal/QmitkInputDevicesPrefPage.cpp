/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkInputDevicesPrefPage.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QHashIterator>
#include <QMessageBox>

#include <mitkIInputDeviceRegistry.h>
#include <mitkIInputDeviceDescriptor.h>
#include <mitkCoreExtConstants.h>

#include "QmitkCommonExtPlugin.h"

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(mitk::CoreExtConstants::INPUTDEVICE_PREFERENCES.toStdString());
  }
}

QmitkInputDevicesPrefPage::QmitkInputDevicesPrefPage()
: m_MainControl(nullptr)
{
}

void QmitkInputDevicesPrefPage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkInputDevicesPrefPage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);
  auto  layout = new QVBoxLayout;

  QList<mitk::IInputDeviceDescriptor::Pointer> temp(GetInputDeviceRegistry()->GetInputDevices());

  for(QList<mitk::IInputDeviceDescriptor::Pointer>::const_iterator it = temp.begin(); it != temp.end();++it)
  {
    QString inputDeviceName((*it)->GetName());
    auto   checkBox = new QCheckBox((inputDeviceName),m_MainControl);
    layout->addWidget(checkBox);
    m_InputDevices.insert(checkBox,(*it)->GetID());

    if(inputDeviceName == "WiiMote")
    {
      m_WiiMoteModes = new QGroupBox("WiiMote Modus");

      m_WiiMoteHeadTracking = new QRadioButton(mitk::CoreExtConstants::WIIMOTE_HEADTRACKING);
      m_WiiMoteSurfaceInteraction = new QRadioButton(mitk::CoreExtConstants::WIIMOTE_SURFACEINTERACTION);
      m_WiiMoteHeadTracking->setChecked(true);

      auto   vBoxLayout = new QVBoxLayout;

      vBoxLayout->addWidget(m_WiiMoteHeadTracking);
      vBoxLayout->addWidget(m_WiiMoteSurfaceInteraction);

      m_WiiMoteModes->setLayout(vBoxLayout);

      layout->addWidget(m_WiiMoteModes);
    }

  }

  layout->addStretch();
  m_MainControl->setLayout(layout);
  this->Update();
}

QWidget* QmitkInputDevicesPrefPage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkInputDevicesPrefPage::PerformOk()
{
  auto* prefs = GetPreferences();
  bool result = true;

  mitk::IInputDeviceRegistry* inputDeviceRegistry = GetInputDeviceRegistry();

  QHashIterator<QCheckBox*, QString> it(m_InputDevices);
  while (it.hasNext())
  {
    it.next();
    mitk::IInputDeviceDescriptor::Pointer inputdevice(inputDeviceRegistry->Find(it.value()));

    if(it.value() == mitk::CoreExtConstants::WIIMOTE_XMLATTRIBUTE_NAME)
    {
      const auto headTracking(m_WiiMoteHeadTracking->text().toStdString());
      const auto surfaceInteraction(m_WiiMoteSurfaceInteraction->text().toStdString());

      prefs->PutBool(headTracking, m_WiiMoteHeadTracking->isChecked());
      prefs->PutBool(surfaceInteraction, m_WiiMoteSurfaceInteraction->isChecked());

      // forced flush of the preferences is needed
      // because otherwise the mitk::WiiMoteActivator class
      // cannot distinguish the two different modes without
      // changing the interface for all input devices
      prefs->Flush();
    }

    if(it.key()->isChecked())
    {
      result &= inputdevice->CreateInputDevice()->RegisterInputDevice();
    }
    else
    {
      result &= inputdevice->CreateInputDevice()->UnRegisterInputDevice();

      // temporary fix, unclean solution:
      // e.g. user activates SpaceNavigator and leaves the
      // the wiimote deactivated, the user will get the warning
      // despite the fact that it has never been activated
      if(it.value() == mitk::CoreExtConstants::WIIMOTE_XMLATTRIBUTE_NAME)
      {
        // until now 2010-09-06 there were some unfixed problems
        // with reconnecting the wiimote after disconnecting it.
        // It was suggested that it might have something to do
        // with the type of stack, that is used for the pairing.
        // MS-Stack for example does not work properly.
        QMessageBox::information(nullptr,"WiiMote supportproblem",
          "A reconnect of the WiiMote is not yet supported! "
          "Please restart the application, if you want to "
          "activate the Wii remote/s again.");
      }
    }

    if(result)
    {
      prefs->PutBool(it.value().toStdString(), it.key()->isChecked());
    }
  }
  return result;
}

void QmitkInputDevicesPrefPage::PerformCancel()
{

}

void QmitkInputDevicesPrefPage::Update()
{
  auto* prefs = GetPreferences();
  QHashIterator<QCheckBox*, QString> it(m_InputDevices);

  while (it.hasNext())
  {
    it.next();
    it.key()->setChecked(prefs->GetBool(it.value().toStdString(), false));
    if(it.value() == mitk::CoreExtConstants::WIIMOTE_XMLATTRIBUTE_NAME)
    {
      m_WiiMoteHeadTracking->setChecked(
        prefs->GetBool(mitk::CoreExtConstants::WIIMOTE_HEADTRACKING.toStdString(), false));
      m_WiiMoteSurfaceInteraction->setChecked
        (prefs->GetBool(mitk::CoreExtConstants::WIIMOTE_SURFACEINTERACTION.toStdString(), false));
    }
  }
}

mitk::IInputDeviceRegistry *QmitkInputDevicesPrefPage::GetInputDeviceRegistry() const
{
  ctkServiceReference serviceRef = QmitkCommonExtPlugin::getContext()->getServiceReference<mitk::IInputDeviceRegistry>();
  if (!serviceRef) return nullptr;

  return QmitkCommonExtPlugin::getContext()->getService<mitk::IInputDeviceRegistry>(serviceRef);
}
