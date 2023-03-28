/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNavigationToolStorageSerializer_h
#define mitkNavigationToolStorageSerializer_h

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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

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
#endif
