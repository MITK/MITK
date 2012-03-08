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

US_DECLARE_SERVICE_INTERFACE(mitk::IDataNodeReader, "org.mitk.IDataNodeReader")

#endif // MITKIDATANODEREADER_H
