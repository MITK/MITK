/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkItkImageIO_h
#define mitkItkImageIO_h

#include "mitkAbstractFileIO.h"
#include <mitkImage.h>
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

    /**Helper function that can be used to convert a MetaDataDictionary into a PropertyList for a certain mimeType.
    The function uses the Property serialization service for that.
    @param mimeTypeName Mime type that should be assumed for the meta data deserialization.
    @param defaultMetaDataKeys Vector of keys that should be assumed as defaults. For defaults no PropertyInfo will be registered
    at the PropertyPersistence service, as they are assumed to be handled anyways. For all other keys an info will be registered
    to ensure that they will be serialized again, even if unkown.
    @param dictionary Reference to the meta data dictionary that contains the information that should be extracted.*/
    static PropertyList::Pointer ExtractMetaDataAsPropertyList(const itk::MetaDataDictionary& dictionary, const std::string& mimeTypeName, const std::vector<std::string>& defaultMetaDataKeys);

    /** Helper function that van be used to extract a raw mitk image for the passed path using the also passed ImageIOBase instance.
    Raw means, that only the pixel data and geometry information is loaded. But e.g. no properties etc...*/
    static Image::Pointer LoadRawMitkImageFromImageIO(itk::ImageIOBase* imageIO, const std::string& path);

    /** Helper function that van be used to extract a raw mitk image for the passed path using the also passed ImageIOBase instance.
    Raw means, that only the pixel data and geometry information is loaded. But e.g. no properties etc...*/
    static void PreparImageIOToWriteImage(itk::ImageIOBase* imageIO, const Image* image);

    static void SavePropertyListAsMetaData(itk::MetaDataDictionary& dictionary, const PropertyList* properties, const std::string& mimeTypeName);


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

#endif
