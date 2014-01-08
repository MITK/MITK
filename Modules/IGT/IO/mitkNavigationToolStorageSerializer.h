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

#ifndef NAVIGATIONTOOLSTORAGESERIALIZER_H_INCLUDED
#define NAVIGATIONTOOLSTORAGESERIALIZER_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include "mitkNavigationToolStorage.h"
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief This class offers methods to save an object of the class NavigationToolStorage
  *        to the harddisc.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationToolStorageSerializer : public itk::Object
  {
  public:
    mitkClassMacro(NavigationToolStorageSerializer,itk::Object);
    itkNewMacro(Self);

    /**
     * @brief  Saves a mitk navigation tool storage to a file.
     * @return Returns true always true since error handling was converted to exception handling.
     *         The return value is decrepated. Will be changed to void.
     * @throw mitk::IGTIOException Throws an exception if the given filename cannot be opened for writing or
     *                             if the temp directory is not accessible.
     */
    bool Serialize(std::string filename, mitk::NavigationToolStorage::Pointer storage);

    /**
     * @brief This method is decrepated. Exceptions are used for error handling now!
     * @return Returns always an empty string since error handling was converted to exception handling.
     */
    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolStorageSerializer();
    ~NavigationToolStorageSerializer();

   std::string m_ErrorMessage;

   std::string convertIntToString(int i);

   std::string m_tempDirectory;

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGESERIALIZER
