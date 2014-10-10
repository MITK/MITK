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
#ifndef _MITK_CONTOURMODEL_READER__H_
#define _MITK_CONTOURMODEL_READER__H_

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkMimeType.h>
#include <mitkContourModel.h>


#include <string>
#include <stack>
#include <vtkXMLParser.h>
#include <tinyxml.h>

namespace mitk
{
/**
 * @brief
 * @ingroup PSIO
 * @ingroup IO
*/
class ContourModelReader : public mitk::AbstractFileReader
{
public:
  ContourModelReader(const ContourModelReader& other);

  ContourModelReader();

  virtual ~ContourModelReader();

  using AbstractFileReader::Read;
  virtual std::vector<itk::SmartPointer<BaseData> > Read();

protected:

  virtual void ReadPoints(mitk::ContourModel::Pointer newContourModel,
    TiXmlElement* currentTimeSeries, unsigned int currentTimeStep);

private:

  ContourModelReader* Clone() const;

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
};

}


#endif
