/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlyFileReaderService_h
#define mitkPlyFileReaderService_h

#include <mitkAbstractFileReader.h>
#include <mitkIOMimeTypes.h>

namespace mitk
{
  class BaseData;

  /**
  * @brief Used to read surfaces from the PLY format.
  *
  * This reader can read binary and ASCII versions of the format transparently.
  *
  * @ingroup IOExt
  */
  class PlyFileReaderService : public AbstractFileReader
  {
  public:
    PlyFileReaderService();
    ~PlyFileReaderService() override;

    using AbstractFileReader::Read;

    static mitk::CustomMimeType mimeType;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    PlyFileReaderService *Clone() const override;
  };

} // namespace mitk

#endif
