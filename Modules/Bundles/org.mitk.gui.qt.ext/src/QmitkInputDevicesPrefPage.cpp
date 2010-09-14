/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
Version:   $Revision: 18019 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkInputDevicesPrefPage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QHashIterator>
#include <QMessageBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <mitkIInputDeviceRegistry.h>
#include <mitkIInputDeviceDescriptor.h>
#include <mitkCoreExtConstants.h>


QmitkInputDevicesPrefPage::QmitkInputDevicesPrefPage()
: m_MainControl(0)
{
  // gets the old setting of the preferences and loads them into the preference node
  berry::IPreferencesService::Pointer prefService 
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  this->m_InputDevicesPrefNode = prefService->GetSystemPreferences()->Node(mitk::CoreExtConstants::INPUTDEVICE_PREFERENCES);
}

void QmitkInputDevicesPrefPage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkInputDevicesPrefPage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout;

  mitk::IInputDeviceRegistry::Pointer inputDeviceRegistry =
    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IInputDeviceRegistry>(mitk::CoreExtConstants::INPUTDEVICE_SERVICE);
  std::vector<mitk::IInputDeviceDescriptor::Pointer> temp(inputDeviceRegistry->GetInputDevices());

  for(std::vector<mitk::IInputDeviceDescriptor::Pointer>::const_iterator it = temp.begin(); it != temp.end();++it)
  {
    QCheckBox* checkBox = new QCheckBox(QString::fromStdString((*it)->GetName()),m_MainControl);
    layout->addWidget(checkBox);
    m_InputDevices.insert(checkBox,(*it)->GetID());
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
  bool result = true;
  mitk::IInputDeviceRegistry::Pointer inputDeviceRegistry =
    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IInputDeviceRegistry>(mitk::CoreExtConstants::INPUTDEVICE_SERVICE);
  QHashIterator<QCheckBox*, std::string> it(m_InputDevices);
  while (it.hasNext())
  {
    it.next();
    mitk::IInputDeviceDescriptor::Pointer inputdevice(inputDeviceRegistry->Find(it.value()));
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
      if(it.value() == "org.mitk.inputdevices.wiimote")
      {
        // until now 2010-09-06 there were some unfixed problems
        // with reconnecting the wiimote after disconnecting it.
        // It was suggested that it might have something to do 
        // with the type of stack, that is used for the pairing.
        // MS-Stack for example does not work properly.
        QMessageBox::information(NULL,"WiiMote supportproblem",
          "A reconnect of the WiiMote is not yet supported! "
          "Please restart the application, if you want to "
          "activate the Wii remote/s again.");
      }
    }

    if(result)
    {
      this->m_InputDevicesPrefNode->PutBool(it.value(),it.key()->isChecked());
    }
  }
  return result;
}

void QmitkInputDevicesPrefPage::PerformCancel()
{

}

void QmitkInputDevicesPrefPage::Update()
{
  QHashIterator<QCheckBox*, std::string> it(m_InputDevices);
  while (it.hasNext())
  {
    it.next();
    it.key()->setChecked(this->m_InputDevicesPrefNode->GetBool(it.value(), false));
  }
}
