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
   * \brief Reader for mitk::ContourModel objects stored in XML format.
   * \ingroup MitkContourModelModule
   */
  class ContourModelReader : public mitk::AbstractFileReader
  {
  public:
    /** \brief Copy constructor. */
    ContourModelReader(const ContourModelReader &other);

    /** \brief Default constructor. Registers reader for the ContourModel MIME type. */
    ContourModelReader();

    ~ContourModelReader() override;

    using AbstractFileReader::Read;

  protected:
    /**
     * \brief Read control points from an XML element and add them to a ContourModel.
     *
     * \param newContourModel The contour model to populate with points.
     * \param currentTimeSeries The XML element containing the point data for one time step.
     * \param currentTimeStep The time step index to which the points belong.
     */
    virtual void ReadPoints(mitk::ContourModel::Pointer newContourModel,
                            const tinyxml2::XMLElement *currentTimeSeries,
                            unsigned int currentTimeStep);

    /**
     * \brief Perform the actual reading of ContourModel data from the file.
     * \return A vector of loaded BaseData objects (ContourModels).
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ContourModelReader *Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };
}

#endif
