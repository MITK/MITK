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

#ifndef FILEWRITERWITHINFORMATION_H_INCLUDED
#define FILEWRITERWITHINFORMATION_H_INCLUDED

#include "mitkFileWriter.h"
#include "mitkBaseData.h"

namespace mitk {
/**
 * \brief Interface for FileWriters with extra information.
 * Should be merged into FileWriter.
 *
 * \deprecatedSince{2014_10} Use mitk::IFileWriter instead.
*/
class FileWriterWithInformation : public FileWriter {
  public:
    mitkClassMacro(FileWriterWithInformation,FileWriter);

    virtual const char *GetDefaultFilename() = 0;
    virtual const char *GetFileDialogPattern() = 0;
    virtual const char *GetDefaultExtension() = 0;
    virtual bool CanWriteBaseDataType(BaseData::Pointer data) = 0;
    virtual void DoWrite(BaseData::Pointer data) = 0;
};
}
#endif
