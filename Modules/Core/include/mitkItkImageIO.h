/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKITKFILEIO_H
#define MITKITKFILEIO_H

#include "mitkAbstractFileIO.h"

#include <itkImageIOBase.h>

namespace mitk
{
  /**
   * This class wraps ITK image IO objects as mitk::IFileReader and
   * mitk::IFileWriter objects.
   *
   * Instantiating this class with a given itk::ImageIOBase instance
   * will register corresponding MITK reader/writer services for that
   * ITK ImageIO object.
   * For all ITK ImageIOs that support the serialization of MetaData
   * (e.g. nrrd or mhd) the ItkImageIO ensures the serialization
   * of Identification UID.
   */
  class MITKCORE_EXPORT ItkImageIO : public AbstractFileIO
  {
  public:
    ItkImageIO(itk::ImageIOBase::Pointer imageIO);
    ItkImageIO(const CustomMimeType &mimeType, itk::ImageIOBase::Pointer imageIO, int rank);

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    virtual std::vector<std::string> FixUpImageIOExtensions(const std::string &imageIOName);
    virtual void FixUpCustomMimeTypeName(const std::string &imageIOName, CustomMimeType &customMimeType);

    // Fills the m_DefaultMetaDataKeys vector with default values
    virtual void InitializeDefaultMetaDataKeys();

    // -------------- AbstractFileReader -------------
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ItkImageIO(const ItkImageIO &other);

    ItkImageIO *IOClone() const override;

    itk::ImageIOBase::Pointer m_ImageIO;

    std::vector<std::string> m_DefaultMetaDataKeys;
  };

  /**Helper function that converts the content of a meta data into a time point vector.
   * If MetaData is not valid or cannot be converted an empty vector is returned.*/
  MITKCORE_EXPORT std::vector<TimePointType> ConvertMetaDataObjectToTimePointList(const itk::MetaDataObjectBase* data);


  /**Helper function that converts the time points of a passed time geometry to a time point list
   and stores it in a itk::MetaDataObject. Use ConvertMetaDataObjectToTimePointList() to convert it back
   to a time point list.*/
  MITKCORE_EXPORT itk::MetaDataObjectBase::Pointer ConvertTimePointListToMetaDataObject(const mitk::TimeGeometry* timeGeometry);

} // namespace mitk

#endif /* MITKITKFILEIO_H */
