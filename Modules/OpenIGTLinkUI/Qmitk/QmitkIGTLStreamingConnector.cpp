/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkIGTLStreamingConnector.h"

//mitk headers
#include <mitkSurface.h>
#include <mitkIGTLDeviceSource.h>
#include <mitkDataStorage.h>
#include <mitkIGTLMessageFactory.h>

//qt headers
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qscrollbar.h>

//igtl
#include <igtlStringMessage.h>
#include <igtlBindMessage.h>
#include <igtlQuaternionTrackingDataMessage.h>
#include <igtlTrackingDataMessage.h>

//poco headers
//#include <Poco/Path.h>

const std::string QmitkIGTLStreamingConnector::VIEW_ID =
    "org.mitk.views.igtldevicesourcemanagementwidget";

const unsigned int
QmitkIGTLStreamingConnector::MILISECONDS_BETWEEN_FPS_CHECK = 50;

QmitkIGTLStreamingConnector::QmitkIGTLStreamingConnector(
    QObject* parent)
  : QObject(parent)
{
}


QmitkIGTLStreamingConnector::~QmitkIGTLStreamingConnector()
{
}

void QmitkIGTLStreamingConnector::Initialize(
    mitk::IGTLMessageSource::Pointer msgSource,
    mitk::IGTLMessageProvider::Pointer msgProvider)
{
  this->m_IGTLMessageProvider = msgProvider;
  this->m_IGTLMessageSource = msgSource;

  connect(&this->m_CheckFPSTimer, SIGNAL(timeout()),
          this, SLOT(OnCheckFPS()));
  connect(&this->m_StreamingTimer, SIGNAL(timeout()),
          this, SLOT(OnUpdateSource()));

  this->m_CheckFPSTimer.start(MILISECONDS_BETWEEN_FPS_CHECK);
}

void QmitkIGTLStreamingConnector::OnCheckFPS()
{
  //get the fps from the source
  unsigned int fps = this->m_IGTLMessageSource->GetFPS();

  //check if the fps is set
  if ( fps > 0 )
  {
    if (!this->m_StreamingTimer.isActive())
    {
      //it is set, so the streaming has to be started
      int ms = 1.0 / (double)fps * 1000;
      this->m_StreamingTimer.start( ms );
    }
  }
  else
  {
    //stop the streaming
    this->m_StreamingTimer.stop();
  }
}

void QmitkIGTLStreamingConnector::OnUpdateSource()
{
  //update the message source
  this->m_IGTLMessageSource->Update();

  //get the latest message
  mitk::IGTLMessage* curMsg = this->m_IGTLMessageSource->GetOutput();

  //check if this message is valid
  if ( curMsg->IsDataValid() )
  {
    //send the latest output to the message provider
    this->m_IGTLMessageProvider->Send(curMsg);
  }
}
