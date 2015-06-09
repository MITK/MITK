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
#ifndef VtkUnstructuredGridReader_H_HEADER_INCLUDED
#define VtkUnstructuredGridReader_H_HEADER_INCLUDED

#include <mitkAbstractFileReader.h>

namespace mitk {
//##Documentation
//## @brief Reader to read unstructured grid files in vtk-format
class VtkUnstructuredGridReader : public AbstractFileReader
{
public:

  VtkUnstructuredGridReader();
  virtual ~VtkUnstructuredGridReader();

  using AbstractFileReader::Read;
  virtual std::vector< itk::SmartPointer<BaseData> > Read() override;

protected:
  virtual VtkUnstructuredGridReader* Clone() const override;

};

} // namespace mitk

#endif /* VtkUnstructuredGridReader_H_HEADER_INCLUDED */
