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

// MITK
#include "mitkNavigationDataSetWriterXML.h"

// Third Party
#include <tinyxml.h>
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <iostream>

mitk::NavigationDataSetWriterXML::NavigationDataSetWriterXML()
{
}

mitk::NavigationDataSetWriterXML::~NavigationDataSetWriterXML()
{
}

void mitk::NavigationDataSetWriterXML::Write (std::string path, mitk::NavigationDataSet::Pointer data)
{
  std::ofstream stream;
  stream.open (path.c_str(), std::ios_base::trunc);

  // Pass to Stream Handler
  Write(&stream, data);
  stream.close();
}

void mitk::NavigationDataSetWriterXML::Write (std::ostream* stream, mitk::NavigationDataSet::Pointer data)
{
  //save old locale
  char * oldLocale;
  oldLocale = setlocale( LC_ALL, 0 );

  StreamHeader(stream, data);
  StreamData(stream, data);
  StreamFooter(stream);

  // Cleanup

  stream->flush();

  //switch back to old locale
  setlocale( LC_ALL, oldLocale );
}

void mitk::NavigationDataSetWriterXML::StreamHeader (std::ostream* stream, mitk::NavigationDataSet::Pointer data)
{
  stream->precision(10);

  //TODO store date and GMT time
  //checking if the stream is good
  if (stream->good())
  {
    *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << std::endl;
    /**m_Stream << "<Version Ver=\"1\" />" << std::endl;*/
    // should be a generic version, meaning a member variable, which has the actual version
    *stream << "    " << "<Data ToolCount=\"" << data->GetNumberOfTools() << "\" version=\"1.0\">" << std::endl;
  }
}

void mitk::NavigationDataSetWriterXML::StreamData (std::ostream* stream, mitk::NavigationDataSet::Pointer data)
{
  // For each time step in the Dataset
  for (mitk::NavigationDataSet::NavigationDataSetIterator it = data->Begin(); it != data->End(); it++)
  {
    for (int toolIndex = 0; toolIndex < it->size(); toolIndex++)
    {
      mitk::NavigationData::Pointer nd = it->at(toolIndex);
      TiXmlElement* elem = new TiXmlElement("ND");

      elem->SetDoubleAttribute("Time", nd->GetIGTTimeStamp());
      // elem->SetAttribute("SystemTime", sysTimeStr); // tag for system time
      elem->SetDoubleAttribute("Tool", toolIndex);
      elem->SetDoubleAttribute("X", nd->GetPosition()[0]);
      elem->SetDoubleAttribute("Y", nd->GetPosition()[1]);
      elem->SetDoubleAttribute("Z", nd->GetPosition()[2]);

      elem->SetDoubleAttribute("QX", nd->GetOrientation()[0]);
      elem->SetDoubleAttribute("QY", nd->GetOrientation()[1]);
      elem->SetDoubleAttribute("QZ", nd->GetOrientation()[2]);
      elem->SetDoubleAttribute("QR", nd->GetOrientation()[3]);

      elem->SetDoubleAttribute("C00", nd->GetCovErrorMatrix()[0][0]);
      elem->SetDoubleAttribute("C01", nd->GetCovErrorMatrix()[0][1]);
      elem->SetDoubleAttribute("C02", nd->GetCovErrorMatrix()[0][2]);
      elem->SetDoubleAttribute("C03", nd->GetCovErrorMatrix()[0][3]);
      elem->SetDoubleAttribute("C04", nd->GetCovErrorMatrix()[0][4]);
      elem->SetDoubleAttribute("C05", nd->GetCovErrorMatrix()[0][5]);
      elem->SetDoubleAttribute("C10", nd->GetCovErrorMatrix()[1][0]);
      elem->SetDoubleAttribute("C11", nd->GetCovErrorMatrix()[1][1]);
      elem->SetDoubleAttribute("C12", nd->GetCovErrorMatrix()[1][2]);
      elem->SetDoubleAttribute("C13", nd->GetCovErrorMatrix()[1][3]);
      elem->SetDoubleAttribute("C14", nd->GetCovErrorMatrix()[1][4]);
      elem->SetDoubleAttribute("C15", nd->GetCovErrorMatrix()[1][5]);

      if (nd->IsDataValid())
        elem->SetAttribute("Valid",1);
      else
        elem->SetAttribute("Valid",0);

      if (nd->GetHasOrientation())
        elem->SetAttribute("hO",1);
      else
        elem->SetAttribute("hO",0);

      if (nd->GetHasPosition())
        elem->SetAttribute("hP",1);
      else
        elem->SetAttribute("hP",0);

      *stream << "        " << *elem << std::endl;

      delete elem;
    }
  }
}

void mitk::NavigationDataSetWriterXML::StreamFooter (std::ostream* stream)
{
  *stream << "</Data>" << std::endl;
}
