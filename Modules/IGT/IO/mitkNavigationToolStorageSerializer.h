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
  class MITKIGT_EXPORT NavigationToolStorageSerializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(NavigationToolStorageSerializer,itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * @brief Saves a mitk navigation tool storage to a file.
     * @throw mitk::IGTIOException Throws an exception if the given filename cannot be opened for writing or
     *                             if the temp directory is not accessible.
     */
    void Serialize(const std::string& filename, mitk::NavigationToolStorage::Pointer storage);

  protected:
    NavigationToolStorageSerializer();
    ~NavigationToolStorageSerializer() override;

   std::string convertIntToString(int i);

   std::string m_tempDirectory;

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGESERIALIZER
