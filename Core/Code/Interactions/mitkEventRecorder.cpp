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

#include "mitkEventRecorder.h"
#include "mitkEventFactory.h"
#include "mitkInteractionEvent.h"


mitk::EventRecorder::EventRecorder()
  : m_Active(false)
{
}

mitk::EventRecorder::~EventRecorder()
{
  if (m_FileStream.is_open())
  {
    m_FileStream.flush();
    m_FileStream.close();
  }
}

void mitk::EventRecorder::Notify(mitk::InteractionEvent *interactionEvent, bool /*isHandled*/)
{
  std::cout << EventFactory::EventToXML(interactionEvent) << "\n";
  if (m_FileStream.is_open())
    m_FileStream << EventFactory::EventToXML(interactionEvent) << "\n";
}

void mitk::EventRecorder::SetEventIgnoreList(std::vector<std::string> list)
{
  m_IgnoreList = list;
}

void mitk::EventRecorder::StartRecording()
{
  if (m_FileName == "")
  {
    MITK_ERROR << "EventRecorder::StartRecording - Filename needs to be set first.";
    return;
  }
  if (m_FileStream.is_open())
  {
    MITK_ERROR << "EventRecorder::StartRecording - Still recording. Stop recording before starting it again.";
    return;
  }

  m_FileStream.open(m_FileName.c_str(), std::ofstream::out );
  if ( !m_FileStream.good() )
  {
    MITK_ERROR << "File " << m_FileName << " could not be opened!";
    m_FileStream.close();
    return ;
  }
}

void mitk::EventRecorder::StopRecording()
{
  if (m_FileStream.is_open())
  {
    m_FileStream.flush();
    m_FileStream.close();
  }
}
