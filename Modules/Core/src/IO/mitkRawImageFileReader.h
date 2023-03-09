/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRawImageFileReader_h
#define mitkRawImageFileReader_h

#include "mitkAbstractFileReader.h"

namespace mitk
{
  /**
   * The user must set the dimensionality, the dimensions and the pixel type.
   * If they are incorrect, the image will not be opened or the visualization will be incorrect.
   */
  class RawImageFileReaderService : public AbstractFileReader
  {
  public:
    /** Supported pixel types. */
    typedef enum { UCHAR, SCHAR, USHORT, SSHORT, UINT, SINT, FLOAT, DOUBLE } IOPixelType;

    /** Endianity of bits. */
    typedef enum { LITTLE, BIG } EndianityType;

    RawImageFileReaderService();

  protected:
    RawImageFileReaderService(const RawImageFileReaderService &other);

    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    using mitk::AbstractFileReader::Read;

  private:
    template <typename TPixel, unsigned int VImageDimensions>
    mitk::BaseData::Pointer TypedRead(const std::string &path, EndianityType endianity, int *size);

    RawImageFileReaderService *Clone() const override;
  };

} // namespace mitk

#endif
