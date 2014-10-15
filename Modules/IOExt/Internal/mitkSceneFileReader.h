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

#ifndef SceneFileReader_H_HEADER_INCLUDED
#define SceneFileReader_H_HEADER_INCLUDED

// MITK
#include <mitkAbstractFileReader.h>

namespace mitk {

class SceneFileReader : public mitk::AbstractFileReader
{

public:

  SceneFileReader();

  using AbstractFileReader::Read;
  virtual std::vector<itk::SmartPointer<BaseData> > Read();
  virtual DataStorage::SetOfObjects::Pointer Read(DataStorage& ds);

private:

  SceneFileReader* Clone() const;
};

} // namespace mitk

#endif /* SceneFileReader_H_HEADER_INCLUDED_C1E7E521 */
