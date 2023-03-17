/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkNavigationDataReaderXML_h
#define mitkNavigationDataReaderXML_h

#include <MitkIGTIOExports.h>

#include <mitkAbstractFileReader.h>
#include <mitkNavigationDataSet.h>
// includes for exceptions
#include <mitkIGTException.h>
#include <mitkIGTIOException.h>

namespace mitk {

  class MITKIGTIO_EXPORT NavigationDataReaderXML : public AbstractFileReader
  {
  public:
    NavigationDataReaderXML();
    ~NavigationDataReaderXML() override;

    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    NavigationDataReaderXML(const NavigationDataReaderXML& other);
    mitk::NavigationDataReaderXML* Clone() const override;

    std::string m_FileName;

  private:
    NavigationDataSet::Pointer Read(std::istream& stream);
    NavigationDataSet::Pointer Read(const std::string& fileName);
  };

} // namespace mitk

#endif
