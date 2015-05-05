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


#ifndef ObjFileReaderService_h
#define ObjFileReaderService_h

#include <mitkAbstractFileReader.h>
#include <mitkIOMimeTypes.h>

namespace mitk {

  class BaseData;


  /**
  * @brief Used to read surfaces from Wavefront OBJ files.
  *
  * @ingroup IOExt
  */
class ObjFileReaderService : public AbstractFileReader
{
public:

  ObjFileReaderService();
  virtual ~ObjFileReaderService();

  using AbstractFileReader::Read;
  virtual std::vector< itk::SmartPointer<BaseData> > Read() override;

  static mitk::CustomMimeType mimeType;

private:

  ObjFileReaderService* Clone() const override;
};

} // namespace mitk

#endif /* ObjFileReaderService_h */
