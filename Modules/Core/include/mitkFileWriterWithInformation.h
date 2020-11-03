/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef FILEWRITERWITHINFORMATION_H_INCLUDED
#define FILEWRITERWITHINFORMATION_H_INCLUDED

#include "mitkBaseData.h"
#include "mitkFileWriter.h"

namespace mitk
{
  /**
   * \brief Interface for FileWriters with extra information.
   * Should be merged into FileWriter.
   *
   * \deprecatedSince{2014_10} Use mitk::IFileWriter instead.
  */
  class FileWriterWithInformation : public FileWriter
  {
  public:
    mitkClassMacro(FileWriterWithInformation, FileWriter);

    virtual const char *GetDefaultFilename() = 0;
    virtual const char *GetFileDialogPattern() = 0;
    virtual const char *GetDefaultExtension() = 0;
    virtual bool CanWriteBaseDataType(BaseData::Pointer data) = 0;
    virtual void DoWrite(BaseData::Pointer data) = 0;
  };
}
#endif
