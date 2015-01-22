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


#ifndef MITKRAWIMAGEFILEREADER_H_
#define MITKRAWIMAGEFILEREADER_H_

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
  typedef enum {UCHAR,SCHAR,USHORT,SSHORT, UINT, SINT, FLOAT, DOUBLE} IOPixelType;

  /** Endianity of bits. */
  typedef enum {LITTLE, BIG} EndianityType;

  RawImageFileReaderService();

protected:

  RawImageFileReaderService(const RawImageFileReaderService& other);

  virtual std::vector<itk::SmartPointer<BaseData> > Read();

  using mitk::AbstractFileReader::Read;

private:

  template <typename TPixel, unsigned int VImageDimensions>
  mitk::BaseData::Pointer TypedRead(const std::string& path, EndianityType endianity, int* size);

  RawImageFileReaderService* Clone() const;

  /** Vector containing dimensions of image to be read. */
  itk::Vector<int, 3> m_Dimensions;

};

} // namespace mitk

#endif /* MITKRAWIMAGEFILEREADER_H_ */
