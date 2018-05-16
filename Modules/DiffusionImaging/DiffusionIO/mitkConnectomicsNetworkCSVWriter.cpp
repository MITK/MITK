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

#include "mitkConnectomicsNetworkCSVWriter.h"
#include "mitkConnectomicsNetworkDefinitions.h"
#include "mitkDiffusionIOMimeTypes.h"
#include <mitkConnectomicsNetworkProperties.h>
#include <mitkStringProperty.h>
#include <boost/algorithm/string.hpp>

mitk::ConnectomicsNetworkCSVWriter::ConnectomicsNetworkCSVWriter()
  : AbstractFileWriter(mitk::ConnectomicsNetwork::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionIOMimeTypes::CONNECTOMICS_LIST_MIMETYPE() ), "Write only CSV Connectomics Networks" )
{
  RegisterService();
}

mitk::ConnectomicsNetworkCSVWriter::ConnectomicsNetworkCSVWriter(const mitk::ConnectomicsNetworkCSVWriter& other)
  : AbstractFileWriter(other)
{
}


mitk::ConnectomicsNetworkCSVWriter::~ConnectomicsNetworkCSVWriter()
{}

mitk::ConnectomicsNetworkCSVWriter* mitk::ConnectomicsNetworkCSVWriter::Clone() const
{
  return new ConnectomicsNetworkCSVWriter(*this);
}

void mitk::ConnectomicsNetworkCSVWriter::Write()
{
  MITK_INFO << "Writing connectomics network";
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
  if (input.IsNull() )
  {
    MITK_ERROR <<"Sorry, input to ConnectomicsNetworkMatrixWriter is nullptr!";
    return;
  }

  this->ValidateOutputLocation();

  std::ostream* out;
  std::ofstream outStream;

  if( this->GetOutputStream() )
  {
    out = this->GetOutputStream();
  }
  else
  {
    outStream.open( this->GetOutputLocation().c_str() );
    out = &outStream;
  }

  if ( !out->good() )
  {
    mitkThrow() << "Could not open stream.";
  }

  try
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, nullptr );
    setlocale(LC_ALL, locale.c_str());

    std::locale previousLocale(out->getloc());
    std::locale I("C");
    out->imbue(I);

    // construct header
    std::stringstream header;

    mitk::StringProperty::Pointer addHeaderInfoProperty = dynamic_cast<mitk::StringProperty*>(input->GetProperty(connectomicsDataAdditionalHeaderInformation.c_str()).GetPointer());
    if(addHeaderInfoProperty.IsNotNull())
    {
      std::string additionalHeaderInfo = addHeaderInfoProperty->GetValue();
      // if the additional header info contains newlines we need to add #
      // in front of the new lines
      std::vector<std::string> strings;
      boost::split(strings, additionalHeaderInfo, boost::is_any_of("\n"));
      for( unsigned int index(0); index < strings.size(); ++index)
      {
        header << "#" << strings[index] << "\n";
      }
    }

    // construct body
    std::stringstream body;

    std::vector< InputType::VertexDescriptorType > nodes = input->GetVectorOfAllVertexDescriptors();
    for(unsigned int i(0); i < nodes.size(); ++i)
    {
      for(unsigned int j(0); j < i; ++j)
      {
        double weight(0);
        if( input->EdgeExists(nodes[i], nodes[j]) )
        {
          weight = (input->GetEdge(nodes[i], nodes[j])).edge_weight;
        }
        body << (input->GetNode(nodes[i])).label << " "
             << (input->GetNode(nodes[j])).label << " "
             << weight << "\n";
      }
    }

    (*out)<< header.str() << body.str();

    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Connectomics network connection list written";
  }
  catch(...)
  {
    mitkThrow() << "Error while writing to stream.";
  }
}

mitk::IFileIO::ConfidenceLevel mitk::ConnectomicsNetworkCSVWriter::GetConfidenceLevel() const
{
  if (AbstractFileWriter::GetConfidenceLevel() == Unsupported)
  {
    return Unsupported;
  }

  // exporting is supported but will result in loss of data
  // which prevents re-import
  return PartiallySupported;
}
