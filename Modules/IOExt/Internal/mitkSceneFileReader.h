/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef SceneFileReader_H_HEADER_INCLUDED
#define SceneFileReader_H_HEADER_INCLUDED

// MITK
#include <mitkAbstractFileReader.h>

namespace mitk
{
  class SceneFileReader : public mitk::AbstractFileReader
  {
  public:
    SceneFileReader();

    using AbstractFileReader::Read;
    DataStorage::SetOfObjects::Pointer Read(DataStorage &ds) override;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SceneFileReader *Clone() const override;
  };

} // namespace mitk

#endif /* SceneFileReader_H_HEADER_INCLUDED_C1E7E521 */
