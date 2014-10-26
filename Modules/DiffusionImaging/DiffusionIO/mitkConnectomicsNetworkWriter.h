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

#ifndef __mitkConnectomicsNetworkWriter_h
#define __mitkConnectomicsNetworkWriter_h

#include <mitkAbstractFileWriter.h>
#include "mitkConnectomicsNetwork.h"
#include <vtkPolyDataWriter.h>


namespace mitk
{

  /**
  * Writes connectomics networks to a file
  * @ingroup Process
  */
  class ConnectomicsNetworkWriter : public mitk::AbstractFileWriter
  {
  public:
    typedef mitk::ConnectomicsNetwork InputType;
    typedef std::vector< std::pair< std::pair<
      mitk::ConnectomicsNetwork::NetworkNode, mitk::ConnectomicsNetwork::NetworkNode >
      , mitk::ConnectomicsNetwork::NetworkEdge > > EdgeVectorType;
    typedef std::vector< mitk::ConnectomicsNetwork::NetworkNode >  VertexVectorType;

    ConnectomicsNetworkWriter();
    virtual ~ConnectomicsNetworkWriter();

    using AbstractFileWriter::Write;
    virtual void Write();

  protected:

    ConnectomicsNetworkWriter(const ConnectomicsNetworkWriter& other);
    virtual mitk::ConnectomicsNetworkWriter* Clone() const;

  };


} // end of namespace mitk

#endif //__mitkConnectomicsNetworkWriter_h
