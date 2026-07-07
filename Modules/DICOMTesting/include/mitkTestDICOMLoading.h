/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkTestDICOMLoading_h
#define mitkTestDICOMLoading_h

#include <mitkDICOMFileReader.h>
#include <mitkPropertyKeyPath.h>

#include <MitkDICOMTestingExports.h>

namespace mitk
{

/** \brief Utility class for testing DICOM file loading and comparing loaded image properties.
 *
 * Provides methods to load DICOM files, dump image metadata to a string representation,
 * and compare two metadata dumps for equality. Used in MITK's DICOM-related test suites
 * to verify that DICOM loading produces expected results.
 *
 * \sa DICOMFileReader
 */
class MITKDICOMTESTING_EXPORT TestDICOMLoading
{
  public:

    typedef std::list<Image::Pointer> ImageList; /**< \brief List of loaded image pointers. */

    TestDICOMLoading();

    /** \brief Load DICOM files and return the resulting images.
     *
     * Uses the internal DICOMFileReader to load the given files and returns
     * a list of loaded Image objects.
     *
     * \param[in] files List of file paths to load.
     * \return A list of loaded images (may be empty if loading fails).
     */
    ImageList
    LoadFiles( const StringList & files );

    /** \brief Verify a cached image by decorating it with DICOM properties.
     *
     * \param[in] files       The original file list used to produce the cached image.
     * \param[in] cachedImage The cached image to verify and decorate.
     * \return The decorated image.
     */
    Image::Pointer
    DecorateVerifyCachedImage( const StringList& files, mitk::Image::Pointer cachedImage );

    /** \brief Verify a cached image using a specific tag cache.
     *
     * \param[in] files       The original file list.
     * \param[in] tagCache    The DICOM tag cache to use for property lookup.
     * \param[in] cachedImage The cached image to verify and decorate.
     * \return The decorated image.
     */
    Image::Pointer
    DecorateVerifyCachedImage( const StringList& files, DICOMTagCache *tagCache, mitk::Image::Pointer cachedImage );

    /** \brief Dump relevant image information to a string for later comparison.
     *
     * Serializes geometry, pixel type, dimensions, and key DICOM properties
     * of the image into a human-readable string format.
     *
     * \param[in] image The image to dump information for.
     * \return A string containing the serialized image information.
     *
     * \sa CompareImageInformationDumps
     */
    std::string
    DumpImageInformation( const Image* image );

    /** \brief Compare two image information dumps for equality.
     *
     * Performs a field-by-field comparison of two dumps produced by
     * DumpImageInformation(), with tolerance for floating-point values.
     *
     * \param[in] reference The reference dump string.
     * \param[in] test      The test dump string to compare against the reference.
     * \return \c true if the dumps are sufficiently equal, \c false otherwise.
     *
     * \sa DumpImageInformation
     */
    bool
    CompareImageInformationDumps( const std::string& reference,
                                  const std::string& test );

  private:

    typedef std::map<std::string,std::string> KeyValueMap;

    DICOMFileReader::Pointer
    BuildDICOMReader();

    void SetDefaultLocale();

    void ResetUserLocale();

    std::string ComponentTypeToString( itk::IOComponentEnum type );

    KeyValueMap ParseDump( const std::string& dump );

    bool CompareSpacedValueFields( const std::string& reference,
                                   const std::string& test,
                                   double eps = mitk::eps );

    bool CompareJSON( const std::string& reference,
                      const std::string& test );

    /**
       Compress whitespace in string
       \param pString input string
       \param pFill replacement whitespace (only whitespace in string after reduction)
       \param pWhitespace characters handled as whitespace
     */
    std::string reduce(const std::string& pString,
                       const std::string& pFill = " ",
                       const std::string& pWhitespace = " \t");

    /**
       Remove leading and trailing whitespace
       \param pString input string
       \param pWhitespace characters handled as whitespace
    */
    std::string trim(const std::string& pString,
                     const std::string& pWhitespace = " \t");

    template<typename T>
    bool StringToNumber(const std::string& s, T& value)
    {
      std::stringstream stream(s);
      stream >> value;
      return (!stream.fail()) && (std::abs(value) <= std::numeric_limits<T>::max());
    }

    static void AddPropertyToDump(const mitk::PropertyKeyPath& key, const mitk::Image* image, std::stringstream& result);

    const char* m_PreviousCLocale;
    std::locale m_PreviousCppLocale;

};

}

#endif
