/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkItkImageIO_h
#define mitkItkImageIO_h

#include <mitkAbstractFileIO.h>
#include <mitkImage.h>
#include <itkImageIOBase.h>

namespace mitk
{
  /**
   * \brief Wraps ITK image IO objects as mitk::IFileReader and mitk::IFileWriter objects.
   *
   * Instantiating this class with a given itk::ImageIOBase instance
   * will register corresponding MITK reader/writer services for that
   * ITK ImageIO object.
   * For all ITK ImageIOs that support the serialization of MetaData
   * (e.g. nrrd or mhd) the ItkImageIO ensures the serialization
   * of Identification UID.
   *
   * \sa AbstractFileIO
   */
  class MITKCORE_EXPORT ItkImageIO : public AbstractFileIO
  {
  public:
    /**
     * \brief Construct an ItkImageIO from an ITK ImageIO instance.
     *
     * \param imageIO The ITK ImageIO base instance to wrap.
     */
    ItkImageIO(itk::ImageIOBase::Pointer imageIO);

    /**
     * \brief Construct an ItkImageIO with a custom MIME type and rank.
     *
     * \param mimeType The custom MIME type for this IO.
     * \param imageIO The ITK ImageIO base instance to wrap.
     * \param rank The service ranking.
     */
    ItkImageIO(const CustomMimeType &mimeType, itk::ImageIOBase::Pointer imageIO, int rank);

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /** \brief Get the confidence level for reading the current file. */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    /** \brief Write the input data to file. */
    void Write() override;

    /** \brief Get the confidence level for writing the current data. */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

    /**
     * \brief Convert a MetaDataDictionary into a PropertyList for a certain MIME type.
     *
     * The function uses the Property serialization service for the conversion.
     *
     * \param dictionary Reference to the meta data dictionary that contains the information to extract.
     * \param mimeTypeName MIME type that should be assumed for the meta data deserialization.
     * \param defaultMetaDataKeys Vector of keys that should be assumed as defaults. For defaults
     *        no PropertyInfo will be registered at the PropertyPersistence service, as they are
     *        assumed to be handled anyways. For all other keys an info will be registered to
     *        ensure that they will be serialized again, even if unknown.
     * \return A PropertyList containing the extracted meta data.
     */
    static PropertyList::Pointer ExtractMetaDataAsPropertyList(const itk::MetaDataDictionary& dictionary, const std::string& mimeTypeName, const std::vector<std::string>& defaultMetaDataKeys);

    /**
     * \brief Extract a raw MITK image from the given file path using the specified ImageIOBase.
     *
     * Raw means that only the pixel data and geometry information is loaded (no properties, etc.).
     *
     * \param imageIO The ITK ImageIOBase instance to use for loading.
     * \param path The file path to read from.
     * \return The loaded MITK image.
     */
    static Image::Pointer LoadRawMitkImageFromImageIO(itk::ImageIOBase* imageIO, const std::string& path);

    /**
     * \brief Prepare an ITK ImageIOBase instance for writing the given MITK image.
     *
     * \param imageIO The ITK ImageIOBase instance to configure.
     * \param image The MITK image to be written.
     */
    static void PreparImageIOToWriteImage(itk::ImageIOBase* imageIO, const Image* image);

    /**
     * \brief Save a PropertyList as meta data in an ITK MetaDataDictionary.
     *
     * \param dictionary The target meta data dictionary.
     * \param properties The PropertyList to save.
     * \param mimeTypeName The MIME type to assume for serialization.
     */
    static void SavePropertyListAsMetaData(itk::MetaDataDictionary& dictionary, const PropertyList* properties, const std::string& mimeTypeName);


  protected:
    /**
     * \brief Fix up file extensions reported by the ITK ImageIO.
     *
     * \param imageIOName The name of the ITK ImageIO.
     * \return A corrected list of file extensions.
     */
    virtual std::vector<std::string> FixUpImageIOExtensions(const std::string &imageIOName);

    /**
     * \brief Fix up the custom MIME type name based on the ITK ImageIO name.
     *
     * \param imageIOName The name of the ITK ImageIO.
     * \param customMimeType The custom MIME type to fix up.
     */
    virtual void FixUpCustomMimeTypeName(const std::string &imageIOName, CustomMimeType &customMimeType);

    /** \brief Fills the m_DefaultMetaDataKeys vector with default values. */
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
