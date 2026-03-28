/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkContourModelReader_h
#define mitkContourModelReader_h

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkContourModel.h>
#include <mitkMimeType.h>

#include <stack>
#include <string>
#include <vtkXMLParser.h>

namespace tinyxml2
{
  class XMLElement;
}

namespace mitk
{
  /**
   * @brief
   * @ingroup MitkContourModelModule
  */
  class ContourModelReader : public mitk::AbstractFileReader
  {
  public:
    ContourModelReader(const ContourModelReader &other);

    ContourModelReader();

    ~ContourModelReader() override;

    using AbstractFileReader::Read;

  protected:
    virtual void ReadPoints(mitk::ContourModel::Pointer newContourModel,
                            const tinyxml2::XMLElement *currentTimeSeries,
                            unsigned int currentTimeStep);
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ContourModelReader *Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };
}

#endif
