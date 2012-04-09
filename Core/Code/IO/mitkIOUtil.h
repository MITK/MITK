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


#ifndef MITKIOUTIL_H
#define MITKIOUTIL_H

#include <MitkExports.h>
#include <mitkDataStorage.h>

namespace mitk {

/**
 * \ingroup IO
 *
 * \brief A utility class to load data from the local file system into a mitk::DataStorage.
 *
 * This class queries the MITK Micro Services registry for registered mitk::IDataNodeReader service
 * instances. The service instance with the highest ranking will be asked first to load the
 * given file. On error (exception thrown) or if no mitk::DataNode was constructed, the next
 * service instance is used.
 *
 * \see mitk::IDataNodeReader
 */
class MITK_CORE_EXPORT IOUtil
{

public:

  /**
   * Load a files in <code>fileNames</code> and add the constructed mitk::DataNode instances
   * to the mitk::DataStorage <code>storage</code>
   *
   * \param fileNames A list (vector) of absolute file name paths.
   * \param storage The data storage to which the constructed data nodes are added.
   * \return The number of added mitk::DataNode instances.
   */
  static int LoadFiles(const std::vector<std::string>&fileNames, DataStorage& storage);

  /**
   * This method will create a new mitk::DataStorage instance and pass it to
   * LoadFiles(std::vector<std::string>,DataStorage).
   *
   * \param fileNames A list (vector) of absolute file name paths.
   * \return The new mitk::DataStorage containing the constructed data nodes.
   *
   * \see LoadFiles(std::vector<std::string>,DataStorage)
   */
  static DataStorage::Pointer LoadFiles(const std::vector<std::string>& fileNames);
};

}

#endif // MITKIOUTIL_H
