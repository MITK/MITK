/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef _MITK_ContourModelSetReader__H_
#define _MITK_ContourModelSetReader__H_

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkMimeType.h>

#include <stack>
#include <string>
#include <vtkXMLParser.h>

namespace mitk
{
  /**
   * @brief
   * @ingroup MitkContourModelModule
  */
  class ContourModelSetReader : public mitk::AbstractFileReader
  {
  public:
    ContourModelSetReader(const ContourModelSetReader &other);

    ContourModelSetReader();

    ~ContourModelSetReader() override;

    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ContourModelSetReader *Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };
}

#endif
