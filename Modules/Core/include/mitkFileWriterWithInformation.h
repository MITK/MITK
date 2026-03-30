/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileWriterWithInformation_h
#define mitkFileWriterWithInformation_h

#include <mitkBaseData.h>
#include <mitkFileWriter.h>

namespace mitk
{
  /**
   * \brief Interface for FileWriters with extra information.
   *
   * Extends FileWriter with methods providing default filename, dialog pattern,
   * and direct write capability for BaseData objects.
   *
   * \deprecatedSince{2014_10} Use mitk::IFileWriter instead.
   *
   * \sa FileWriter
   * \sa IFileWriter
   */
  class FileWriterWithInformation : public FileWriter
  {
  public:
    mitkClassMacro(FileWriterWithInformation, FileWriter);

    /**
     * \brief Get the default filename for this writer.
     *
     * \return The default filename as a C string.
     */
    virtual const char *GetDefaultFilename() = 0;

    /**
     * \brief Get the file dialog filter pattern (e.g. "Images (*.png *.jpg)").
     *
     * \return The file dialog pattern as a C string.
     */
    virtual const char *GetFileDialogPattern() = 0;

    /**
     * \brief Get the default file extension for this writer.
     *
     * \return The default file extension as a C string.
     */
    virtual const char *GetDefaultExtension() = 0;

    /**
     * \brief Check whether this writer can write the given BaseData type.
     *
     * \param data The BaseData to check.
     * \return \c true if the writer supports this data type, \c false otherwise.
     */
    virtual bool CanWriteBaseDataType(BaseData::Pointer data) = 0;

    /**
     * \brief Write the given BaseData to a file.
     *
     * \param data The BaseData to write.
     */
    virtual void DoWrite(BaseData::Pointer data) = 0;
  };
}
#endif
