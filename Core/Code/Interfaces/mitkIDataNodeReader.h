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


#ifndef MITKIDATANODEREADER_H
#define MITKIDATANODEREADER_H

#include <mitkDataNode.h>
#include <mitkServiceInterface.h>

namespace mitk {

class DataStorage;

/**
 * \ingroup MicroServices_Interfaces
 *
 * This interface provides methods to load data from the local filesystem
 * into a given mitk::DataStorage.
 *
 * \deprecatedSince{2014_10} Use mitk::IFileReader instead
 */
struct IDataNodeReader
{
  virtual ~IDataNodeReader() {}

  /**
   * Reads the local file given by <code>fileName</code> and constructs one or more
   * mitk::DataNode instances which are added to the given mitk::DataStorage <code>storage</code>.
   *
   * \param fileName The absolute path to a local file.
   * \param storage The mitk::DataStorage which will contain the constructed data nodes.
   * \return The number of constructed mitk::DataNode instances.
   *
   * \note Errors during reading the file or constructing the data node should be expressed by
   *       throwing appropriate exceptions.
   *
   * \see mitk::DataNodeFactory
   */
  virtual int Read(const std::string& fileName, mitk::DataStorage& storage) = 0;
};

}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IDataNodeReader, "org.mitk.IDataNodeReader")

#endif // MITKIDATANODEREADER_H
