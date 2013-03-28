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


#ifndef FileWriterInterface_H_HEADER_INCLUDED_C1E7E521
#define FileWriterInterface_H_HEADER_INCLUDED_C1E7E521

#include <MitkExports.h>
#include <mitkBaseData.h>
#include <mitkFileWriterInterface.h>

namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class MITK_CORE_EXPORT FileWriterInterface
{
  public:

/**
*
**/

    void Write(std::string path, mitk::BaseData::Pointer data);

    template <class T>
    T* Read(std::string path);

/**
* Returns a compatible Reader to the given file extension
**/
    mitk::FileWriterInterface::Pointer GetWriter(mitk::BaseData::Pointer datatype);

    mitk::FileWriterInterface::Pointer GetWriter(std::string extension, std::list<std::string> properties );

    mitk::FileWriterInterface::Pointer GetWriters(std::string extension);

    mitk::FileWriterInterface::Pointer GetWriters(std::string extension, std::list<std::string> properties );

  protected:
    FileWriterInterface();
    virtual ~FileWriterInterface();

};
} // namespace mitk
#endif /* FileWriterInterface_H_HEADER_INCLUDED_C1E7E521 */
