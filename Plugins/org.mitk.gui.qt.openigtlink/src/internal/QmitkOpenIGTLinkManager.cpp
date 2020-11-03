/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

//
#include "QmitkOpenIGTLinkManager.h"


const std::string QmitkOpenIGTLinkManager::VIEW_ID =
    "org.mitk.views.openigtlinkmanager";

QmitkOpenIGTLinkManager::QmitkOpenIGTLinkManager()
: QmitkAbstractView()
{
}

QmitkOpenIGTLinkManager::~QmitkOpenIGTLinkManager()
{
  for(unsigned int i=0; i < m_AllSourcesHandledByThisWidget.size(); i++)
    m_AllSourcesHandledByThisWidget.at(i)->UnRegisterMicroservice();
}

void QmitkOpenIGTLinkManager::SetFocus()
{
}

void QmitkOpenIGTLinkManager::CreateQtPartControl( QWidget *parent )
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


void QmitkOpenIGTLinkManager::NewSourceByWidget(
    mitk::IGTLDeviceSource::Pointer source,std::string /*sourceName*/)
{
  source->RegisterAsMicroservice(/*sourceName*/);
  m_AllSourcesHandledByThisWidget.push_back(source);
}

void QmitkOpenIGTLinkManager::SourceSelected(
    mitk::IGTLDeviceSource::Pointer source)
{
  if (source.IsNull()) //no source selected
  {
    //reset everything
    return;
  }

  m_CurrentIGTLDevice = source->GetIGTLDevice();
  this->m_Controls.m_SourceManagerWidget->LoadSource(source);

  //check if the current selected source is also a message provider
  mitk::IGTLMessageProvider::Pointer currentMsgProvider =
      mitk::IGTLMessageProvider::New();
  currentMsgProvider = dynamic_cast<mitk::IGTLMessageProvider*>(source.GetPointer());
  if(currentMsgProvider.IsNull())
    return;
  this->m_Controls.m_StreamManagerWidget->LoadSource(currentMsgProvider);
}
