/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkVtkUnstructuredGridReader_h
#define mitkVtkUnstructuredGridReader_h

#include <mitkAbstractFileReader.h>

namespace mitk
{
  /**
   * \brief Reader for unstructured grid files in VTK format.
   */
  class VtkUnstructuredGridReader : public AbstractFileReader
  {
  public:
    VtkUnstructuredGridReader();
    ~VtkUnstructuredGridReader() override;

    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    VtkUnstructuredGridReader *Clone() const override;
  };

} // namespace mitk

#endif
