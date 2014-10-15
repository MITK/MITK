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
#include "mitkInteractionEventConst.h"

#include "mitkBaseRenderer.h"


static void WriteEventXMLHeader(std::ofstream& stream)
{
  stream << mitk::InteractionEventConst::xmlHead() << "\n";
}


static void WriteEventXMLConfig(std::ofstream& stream)
{
  // <config>
  stream << " <" << mitk::InteractionEventConst::xmlTagConfigRoot() << ">\n";

  //write renderer config
  //for all registered 2D renderers write name and viewdirection.
  mitk::BaseRenderer::BaseRendererMapType::iterator rendererIterator = mitk::BaseRenderer::baseRendererMap.begin();
  mitk::BaseRenderer::BaseRendererMapType::iterator end = mitk::BaseRenderer::baseRendererMap.end();

  for(; rendererIterator != end; rendererIterator++)
  {
    std::string rendererName = (*rendererIterator).second->GetName();

    mitk::SliceNavigationController::ViewDirection viewDirection = (*rendererIterator).second->GetSliceNavigationController()->GetDefaultViewDirection();
    mitk::BaseRenderer::MapperSlotId mapperID = (*rendererIterator).second->GetMapperID();

    //  <renderer RendererName="stdmulti.widget2" ViewDirection="1" MapperID="1" SizeX="200" SizeY="200" SizeZ="1"/>
    stream << "  <" << mitk::InteractionEventConst::xmlTagRenderer() << " "
           << mitk::InteractionEventConst::xmlEventPropertyRendererName() << "=\""<<  rendererName << "\" "
           << mitk::InteractionEventConst::xmlEventPropertyViewDirection() << "=\""<< viewDirection << "\" "
           << mitk::InteractionEventConst::xmlEventPropertyMapperID() << "=\"" << mapperID << "\" "
           << mitk::InteractionEventConst::xmlRenderSizeX() << "=\"" << (*rendererIterator).second->GetSize()[0] << "\" "
           << mitk::InteractionEventConst::xmlRenderSizeY() << "=\"" << (*rendererIterator).second->GetSize()[1] << "\" "
           << mitk::InteractionEventConst::xmlRenderSizeZ() << "=\"" << (*rendererIterator).second->GetSize()[2] << "\" "
           << "/>\n";
  }

  // </config>
  stream << " </" << mitk::InteractionEventConst::xmlTagConfigRoot() << ">\n";
}

static void WriteEventXMLEventsOpen(std::ofstream& stream)
{
  stream << " <" << mitk::InteractionEventConst::xmlTagEvents() << ">\n";
}


static void WriteEventXMLEventsClose(std::ofstream& stream)
{
  stream << " </" << mitk::InteractionEventConst::xmlTagEvents() << ">\n";
}


static void WriteEventXMLInteractionsOpen(std::ofstream& stream)
{
  stream << "<" << mitk::InteractionEventConst::xmlTagInteractions() << ">\n";
}


static void WriteEventXMLInteractionsClose(std::ofstream& stream)
{
  stream << "</" << mitk::InteractionEventConst::xmlTagInteractions() << ">";
}


static void WriteEventXMLClose(std::ofstream& stream)
{
  WriteEventXMLEventsClose(stream);
  WriteEventXMLInteractionsClose(stream);
}


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
    return;
  }

  m_Active = true;

  //write head and config
  // <?xml version="1.0"?>
  //  <interactions>
  //   <config>
  //    <renderer RendererName="stdmulti.widget2" ViewDirection="1"/>
  //    <renderer RendererName="stdmulti.widget1" ViewDirection="0"/>
  //     ...
  //   </config>
  //   <events>
  WriteEventXMLHeader(m_FileStream);
  WriteEventXMLInteractionsOpen(m_FileStream);
  WriteEventXMLConfig(m_FileStream);
  WriteEventXMLEventsOpen(m_FileStream);
}

void mitk::EventRecorder::StopRecording()
{
  if (m_FileStream.is_open())
  {
    //write end tag
    //  </events>
    // </interactions>
    WriteEventXMLClose(m_FileStream);

    m_FileStream.flush();
    m_FileStream.close();

    m_Active =false;
  }
}
