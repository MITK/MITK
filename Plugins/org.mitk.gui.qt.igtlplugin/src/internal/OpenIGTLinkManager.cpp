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
}


void OpenIGTLinkManager::NewSourceByWidget(
    mitk::IGTLDeviceSource::Pointer source,std::string /*sourceName*/)
{
  source->RegisterAsMicroservice(/*sourceName*/);
  m_AllSourcesHandledByThisWidget.push_back(source);
}

void OpenIGTLinkManager::SourceSelected(
    mitk::IGTLDeviceSource::Pointer source)
{
  if (source.IsNull()) //no source selected
  {
    //reset everything
    return;
  }

  this->m_Controls.m_SourceManagerWidget->LoadSource(source);

  //check if the current selected source is also a message provider
  mitk::IGTLMessageProvider::Pointer msgProvider =
      mitk::IGTLMessageProvider::New();
  msgProvider = dynamic_cast<mitk::IGTLMessageProvider*>(source.GetPointer());
  this->m_Controls.m_StreamManagerWidget->LoadSource(msgProvider);
}
