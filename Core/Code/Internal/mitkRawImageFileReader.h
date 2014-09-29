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
class RawImageFileReader : public AbstractFileReader
{

public:

  /** Supported pixel types. */
  typedef enum {UCHAR,SCHAR,USHORT,SSHORT, UINT, SINT, FLOAT, DOUBLE} IOPixelType;

  /** Endianity of bits. */
  typedef enum {LITTLE, BIG} EndianityType;

  RawImageFileReader();

protected:

  RawImageFileReader(const RawImageFileReader& other);

  virtual std::vector<itk::SmartPointer<BaseData> > Read();

  using mitk::AbstractFileReader::Read;

private:

  template <typename TPixel, unsigned int VImageDimensions>
  mitk::BaseData::Pointer TypedRead(const std::string& path, EndianityType endianity, int* size);

  RawImageFileReader* Clone() const;

  /** Pixel type of image to be read. Must be of type IOPixelType. */
  IOPixelType m_PixelType;

  /** Dimensionality of file to be read. Can be 2 or 3. */
  int m_Dimensionality;

  /** Endianity. Must be set to LITTLE or BIG. Default is BIG. */
  EndianityType m_Endianity;

  /** Vector containing dimensions of image to be read. */
  itk::Vector<int, 3> m_Dimensions;

};

} // namespace mitk

#endif /* MITKRAWIMAGEFILEREADER_H_ */
