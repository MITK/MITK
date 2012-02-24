/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "DicomEventHandler.h"
#include <service/event/ctkEventConstants.h>
#include <ctkDictionary.h>

DicomEventHandler::DicomEventHandler() 
{    
}

DicomEventHandler::~DicomEventHandler()
{
}

void DicomEventHandler::handleEvent(const ctkEvent& ctkEvent)
{
    std::cout << "I'll handle your event:" ctkEvent.getProperty("SeriesName").toString();
}

void DicomEventHandler::Register(QString eventTopic, QString filter)
{
    ctkDictionary properties;
    properties[ctkEventConstants::EVENT_TOPIC] = eventTopic;
    if(!filter.isEmpty())
    {
        properties[ctkEventConstants::EVENT_FILTER] = filter;
    }
    m_PluginContext->registerService<ctkEventHandler>(this, properties);
}

void DicomEventHandler::SetPluginContext(ctkPluginContext* context)
{
    m_PluginContext = context;
}