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

#ifndef __mitkConnectomicsNetworkCSVWriter_h
#define __mitkConnectomicsNetworkCSVWriter_h

#include <mitkAbstractFileWriter.h>
#include "mitkConnectomicsNetwork.h"
#include <vtkPolyDataWriter.h>


namespace mitk
{

  /**
  * Writes connectomics networks to a file
  *
  * This export is write only, information is lost.
  * @ingroup Process
  */
  class ConnectomicsNetworkCSVWriter : public mitk::AbstractFileWriter
  {
  public:
    typedef mitk::ConnectomicsNetwork InputType;
    typedef std::vector< std::pair< std::pair<
      mitk::ConnectomicsNetwork::NetworkNode, mitk::ConnectomicsNetwork::NetworkNode >
      , mitk::ConnectomicsNetwork::NetworkEdge > > EdgeVectorType;
    typedef std::vector< mitk::ConnectomicsNetwork::NetworkNode >  VertexVectorType;

    ConnectomicsNetworkCSVWriter();
    virtual ~ConnectomicsNetworkCSVWriter();

    using AbstractFileWriter::Write;
    virtual void Write();

    mitk::IFileIO::ConfidenceLevel GetConfidenceLevel() const;

  protected:

    ConnectomicsNetworkCSVWriter(const ConnectomicsNetworkCSVWriter& other);
    virtual mitk::ConnectomicsNetworkCSVWriter* Clone() const;

  };


} // end of namespace mitk

#endif //__mitkConnectomicsNetworkCSVWriter_h
