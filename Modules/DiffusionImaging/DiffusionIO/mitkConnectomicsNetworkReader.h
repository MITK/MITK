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

#ifndef __mitkConnectomicsNetworkReader_h
#define __mitkConnectomicsNetworkReader_h

#include "mitkCommon.h"
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkMimeType.h>
#include "mitkConnectomicsNetwork.h"

namespace mitk
{

  /** \brief The reader for connectomics network files (.cnf)
  */

  class ConnectomicsNetworkReader : public mitk::AbstractFileReader
  {
  public:

    typedef mitk::ConnectomicsNetwork OutputType;

    ConnectomicsNetworkReader(const ConnectomicsNetworkReader& other);
    ConnectomicsNetworkReader();
    virtual ~ConnectomicsNetworkReader();

    using AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read();

  private:
    ConnectomicsNetworkReader* Clone() const;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkConnectomicsNetworkReader_h
