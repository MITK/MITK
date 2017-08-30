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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

//
#include "OpenIGTLinkManager.h"


const std::string OpenIGTLinkManager::VIEW_ID =
    "org.mitk.views.openigtlinkmanager";

OpenIGTLinkManager::OpenIGTLinkManager()
: QmitkAbstractView()
{
}

OpenIGTLinkManager::~OpenIGTLinkManager()
{
  for(unsigned int i=0; i < m_AllSourcesHandledByThisWidget.size(); i++)
    m_AllSourcesHandledByThisWidget.at(i)->UnRegisterMicroservice();
}

void OpenIGTLinkManager::SetFocus()
{
}

void OpenIGTLinkManager::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // create GUI widgets from the Qt Designer's .ui file
//  connect( (QObject*)(m_Controls.m_SourceManagerWidget),
//           SIGNAL(NewSourceAdded(mitk::IGTLDeviceSource::Pointer, std::string)),
//           this,
//           SLOT(NewSourceByWidget(mitk::IGTLDeviceSource::Pointer,std::string)) );
  connect( (QObject*)(m_Controls.m_SourceListWidget),
           SIGNAL(IGTLDeviceSourceSelected(mitk::IGTLDeviceSource::Pointer)),
           this,
           SLOT(SourceSelected(mitk::IGTLDeviceSource::Pointer)) );
  connect(m_Controls.checkBoxLogMessages, SIGNAL(clicked()), this, SLOT(LogMessagesClicked()));

  logMessages = m_Controls.checkBoxLogMessages->isChecked();
}


void OpenIGTLinkManager::NewSourceByWidget(
    mitk::IGTLDeviceSource::Pointer source,std::string /*sourceName*/)
{
  source->RegisterAsMicroservice(/*sourceName*/);
  m_AllSourcesHandledByThisWidget.push_back(source);
}

void OpenIGTLinkManager::LogMessagesClicked()
{
  logMessages = m_Controls.checkBoxLogMessages->isChecked();
  if(m_CurrentIGTLDevice.IsNotNull())
    m_CurrentIGTLDevice->SetLogMessages(logMessages);
  else
    MITK_WARN << "Logging information not passed down to Message Provider.";
}

void OpenIGTLinkManager::SourceSelected(
    mitk::IGTLDeviceSource::Pointer source)
{
  if (source.IsNull()) //no source selected
  {
    //reset everything
    return;
  }

  m_CurrentIGTLDevice = source->GetIGTLDevice();
  m_CurrentIGTLDevice->SetLogMessages(logMessages);
  this->m_Controls.m_SourceManagerWidget->LoadSource(source);

  //check if the current selected source is also a message provider
  mitk::IGTLMessageProvider::Pointer currentMsgProvider =
      mitk::IGTLMessageProvider::New();
  currentMsgProvider = dynamic_cast<mitk::IGTLMessageProvider*>(source.GetPointer());
  if(currentMsgProvider.IsNull())
    return;
  this->m_Controls.m_StreamManagerWidget->LoadSource(currentMsgProvider);
}
