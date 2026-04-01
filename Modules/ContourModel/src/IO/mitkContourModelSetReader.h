/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkContourModelSetReader_h
#define mitkContourModelSetReader_h

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
   * \brief Reader for mitk::ContourModelSet objects stored in XML format.
   * \ingroup MitkContourModelModule
   */
  class ContourModelSetReader : public mitk::AbstractFileReader
  {
  public:
    /** \brief Copy constructor. */
    ContourModelSetReader(const ContourModelSetReader &other);

    /** \brief Default constructor. Registers reader for the ContourModelSet MIME type. */
    ContourModelSetReader();

    ~ContourModelSetReader() override;

    using AbstractFileReader::Read;

  protected:
    /**
     * \brief Perform the actual reading of ContourModelSet data from the file.
     * \return A vector of loaded BaseData objects (ContourModelSets).
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ContourModelSetReader *Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };
}

#endif
