/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkNavigationDataSetWriterXML.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>

// Third Party
#include <tinyxml2.h>
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <iostream>

mitk::NavigationDataSetWriterXML::NavigationDataSetWriterXML() : AbstractFileWriter(NavigationDataSet::GetStaticNameOfClass(),
  mitk::IGTMimeTypes::NAVIGATIONDATASETXML_MIMETYPE(),
  "MITK NavigationDataSet Writer (XML)")
{
  RegisterService();
}

mitk::NavigationDataSetWriterXML::NavigationDataSetWriterXML(const mitk::NavigationDataSetWriterXML& other) : AbstractFileWriter(other)
{
}

mitk::NavigationDataSetWriterXML::~NavigationDataSetWriterXML()
{
}

mitk::NavigationDataSetWriterXML* mitk::NavigationDataSetWriterXML::Clone() const
{
  return new NavigationDataSetWriterXML(*this);
}

void mitk::NavigationDataSetWriterXML::Write()
{
  std::ostream* out = GetOutputStream();
  if (out == nullptr)
  {
    out = new std::ofstream( GetOutputLocation().c_str() );
  }
  mitk::NavigationDataSet::ConstPointer data = dynamic_cast<const NavigationDataSet*> (this->GetInput());

  mitk::LocaleSwitch localeSwitch("C");

  StreamHeader(out, data);
  StreamData(out, data);
  StreamFooter(out);

  // Cleanup
  out->flush();
  delete out;
}

void mitk::NavigationDataSetWriterXML::StreamHeader (std::ostream* stream, mitk::NavigationDataSet::ConstPointer data)
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

void mitk::NavigationDataSetWriterXML::StreamData (std::ostream* stream, mitk::NavigationDataSet::ConstPointer data)
{
  // For each time step in the Dataset
  for (auto it = data->Begin(); it != data->End(); it++)
  {
    for (std::size_t toolIndex = 0; toolIndex < it->size(); toolIndex++)
    {
      mitk::NavigationData::Pointer nd = it->at(toolIndex);
      tinyxml2::XMLDocument doc;
      auto *elem = doc.NewElement("ND");

      elem->SetAttribute("Time", nd->GetIGTTimeStamp());
      // elem->SetAttribute("SystemTime", sysTimeStr); // tag for system time
      elem->SetAttribute("Tool", static_cast<int>(toolIndex));
      elem->SetAttribute("X", nd->GetPosition()[0]);
      elem->SetAttribute("Y", nd->GetPosition()[1]);
      elem->SetAttribute("Z", nd->GetPosition()[2]);

      elem->SetAttribute("QX", nd->GetOrientation()[0]);
      elem->SetAttribute("QY", nd->GetOrientation()[1]);
      elem->SetAttribute("QZ", nd->GetOrientation()[2]);
      elem->SetAttribute("QR", nd->GetOrientation()[3]);

      elem->SetAttribute("C00", nd->GetCovErrorMatrix()[0][0]);
      elem->SetAttribute("C01", nd->GetCovErrorMatrix()[0][1]);
      elem->SetAttribute("C02", nd->GetCovErrorMatrix()[0][2]);
      elem->SetAttribute("C03", nd->GetCovErrorMatrix()[0][3]);
      elem->SetAttribute("C04", nd->GetCovErrorMatrix()[0][4]);
      elem->SetAttribute("C05", nd->GetCovErrorMatrix()[0][5]);
      elem->SetAttribute("C10", nd->GetCovErrorMatrix()[1][0]);
      elem->SetAttribute("C11", nd->GetCovErrorMatrix()[1][1]);
      elem->SetAttribute("C12", nd->GetCovErrorMatrix()[1][2]);
      elem->SetAttribute("C13", nd->GetCovErrorMatrix()[1][3]);
      elem->SetAttribute("C14", nd->GetCovErrorMatrix()[1][4]);
      elem->SetAttribute("C15", nd->GetCovErrorMatrix()[1][5]);

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

      tinyxml2::XMLPrinter printer;
      doc.Print(&printer);

      *stream << "        " << printer.CStr() << std::endl;
    }
  }
}

void mitk::NavigationDataSetWriterXML::StreamFooter (std::ostream* stream)
{
  *stream << "</Data>" << std::endl;
}
