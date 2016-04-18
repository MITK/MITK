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

#include "mitkUSNavigationExperimentLogging.h"
#include "mitkDataNode.h"

#include "itkRealTimeClock.h"

#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/XML/XMLWriter.h"

mitk::USNavigationExperimentLogging::USNavigationExperimentLogging()
  : m_CurrentResultNumber(0),
    m_RealTimeClock(itk::RealTimeClock::New())
{
  this->Reset();
}

mitk::USNavigationExperimentLogging::~USNavigationExperimentLogging()
{
}

void mitk::USNavigationExperimentLogging::Reset()
{
  m_StartTime = m_RealTimeClock->GetTimeInSeconds();

  m_OutputXML = new Poco::XML::Document();
  m_OutputXMLRoot = m_OutputXML->createElement("ExperimentResults");
  m_OutputXML->appendChild(m_OutputXMLRoot);
}

void mitk::USNavigationExperimentLogging::SetFileName(const std::string& fileName)
{
  m_FileName = fileName;
}

void mitk::USNavigationExperimentLogging::SetResult(const itk::SmartPointer<mitk::DataNode> resultNode)
{
  Poco::AutoPtr<Poco::XML::Element> currentResultElement = m_OutputXML->createElement(resultNode->GetName());
  m_OutputXMLRoot->appendChild(currentResultElement);

  this->AddCurrentTimeAttributes(currentResultElement);

  const mitk::PropertyList::PropertyMap* propertyMap = resultNode->GetPropertyList()->GetMap();

  for ( mitk::PropertyList::PropertyMap::const_iterator it = propertyMap->begin();
    it != propertyMap->end(); ++it )
  {
    size_t prefixSize = m_KeyPrefix.size();

    // only write properties with keys begining with the prefix (if a prefix was set)
    if ( ! m_KeyPrefix.empty() && it->first.substr(0, prefixSize) != m_KeyPrefix ) { continue; }

    if ( prefixSize >= it->first.size() )
    {
      MITK_ERROR("USNavigationExperimentLogging") << "Property key must contain more characters then the key prefix.";
      mitkThrow() << "Property key must contain more characters then the key prefix.";
    }

    // create a xml element named like the key but without the prefix and
    // append it into the xml tree
    Poco::AutoPtr<Poco::XML::Element> mapElement = m_OutputXML->createElement(it->first.substr(prefixSize, it->first.size() - prefixSize));
    currentResultElement->appendChild(mapElement);

    // get the class of the property and save it in the tree
    mapElement->setAttribute("class", it->second->GetNameOfClass());

    // convert the value of the property to a string and save it in the tree
    Poco::AutoPtr<Poco::XML::Text> value = m_OutputXML->createTextNode(it->second->GetValueAsString());
    mapElement->appendChild(value);
  }

  this->WriteXmlToFile();
}

void mitk::USNavigationExperimentLogging::AddCurrentTimeAttributes(Poco::AutoPtr<Poco::XML::Element> element)
{
  // get the current time and add it as attribute to the xml
  std::stringstream stringStreamTime;
  stringStreamTime.precision(2);
  stringStreamTime << std::fixed << m_RealTimeClock->GetTimeInSeconds();
  element->setAttribute("time", stringStreamTime.str());

  // get the duration since the start time and add it as attribute to the xml
  std::stringstream stringStreamExperimentTime;
  stringStreamExperimentTime.precision(2);
  stringStreamExperimentTime << std::fixed << m_RealTimeClock->GetTimeInSeconds() - m_StartTime;
  element->setAttribute("duration", stringStreamExperimentTime.str());
}

void mitk::USNavigationExperimentLogging::WriteXmlToFile()
{
  // open file and write tree if successfull
  std::ofstream filestream(m_FileName.c_str());
  if ( filestream.is_open() )
  {
    // print xml tree to the file
    Poco::XML::DOMWriter domWriter;
    domWriter.setNewLine("\n");
    domWriter.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);

    try
    {
      domWriter.writeNode(filestream, m_OutputXML);
    }
    catch (const Poco::Exception &e)
    {
      MITK_ERROR("USNavigationExperimentLogging") << "Cannot write XML tree (" << e.displayText() << ").";
      mitkThrow() << "Cannot write XML tree (" << e.displayText() << ").";
    }

    filestream.close();
  }
  else
  {
    MITK_ERROR("USNavigationExperimentLogging") << "Cannot open output file (" << m_FileName << "). Nothing will be stored at the file system.";
    mitkThrow() << "Cannot open output file (" << m_FileName << "). Nothing will be stored at the file system.";
  }
}
