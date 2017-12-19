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

#include "mitkConnectomicsNetworkMatrixWriter.h"
#include "mitkConnectomicsNetworkDefinitions.h"
#include "mitkDiffusionIOMimeTypes.h"
#include <mitkConnectomicsNetworkProperties.h>
#include <boost/algorithm/string.hpp>

mitk::ConnectomicsNetworkMatrixWriter::ConnectomicsNetworkMatrixWriter()
  : AbstractFileWriter(mitk::ConnectomicsNetwork::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionIOMimeTypes::CONNECTOMICS_MATRIX_MIMETYPE() ), "Write only Matrix Connectomics Networks" )
{
  RegisterService();
}

mitk::ConnectomicsNetworkMatrixWriter::ConnectomicsNetworkMatrixWriter(const mitk::ConnectomicsNetworkMatrixWriter& other)
  : AbstractFileWriter(other)
{
}


mitk::ConnectomicsNetworkMatrixWriter::~ConnectomicsNetworkMatrixWriter()
{}

mitk::ConnectomicsNetworkMatrixWriter* mitk::ConnectomicsNetworkMatrixWriter::Clone() const
{
  return new ConnectomicsNetworkMatrixWriter(*this);
}

void mitk::ConnectomicsNetworkMatrixWriter::Write()
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

    // add the order of labels to the header
    std::vector< InputType::VertexDescriptorType > nodes = input->GetVectorOfAllVertexDescriptors();
    header << "#";
    for(unsigned int index(0); index < nodes.size(); ++index)
    {
      header << " " << (input->GetNode(nodes[index])).label;
    }

    // construct body
    std::stringstream body;

    for(unsigned int i(0); i < nodes.size(); ++i)
    {
      for(unsigned int j(0); j < nodes.size(); ++j)
      {
        if( input->EdgeExists(nodes[i], nodes[j]))
        {
        body << (input->GetEdge(nodes[i], nodes[j])).fiber_count;
        }
        else
        {
          body << 0;
        }
        if(j < nodes.size() - 1)
        {
          body << " ";
        }
      }
      body << "\n";
    }

    (*out)<< header.str() << "\n" << body.str();
    setlocale(LC_ALL, currLocale.c_str());
    MITK_INFO << "Connectomics network connection list written";
  }
  catch(...)
  {
    mitkThrow() << "Error while writing to stream.";
  }
}

mitk::IFileIO::ConfidenceLevel mitk::ConnectomicsNetworkMatrixWriter::GetConfidenceLevel() const
{
  if (AbstractFileWriter::GetConfidenceLevel() == Unsupported)
  {
    return Unsupported;
  }

  // exporting is supported but will result in loss of data
  // which prevents re-import
  return PartiallySupported;
}
