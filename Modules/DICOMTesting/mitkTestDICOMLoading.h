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
#ifndef mitkTestDICOMLoading_h
#define mitkTestDICOMLoading_h

#include "mitkClassicDICOMSeriesReader.h"

#include "MitkDICOMTestingExports.h"

namespace mitk
{

class MitkDICOMTesting_EXPORT TestDICOMLoading
{
  public:

    typedef std::list<Image::Pointer> ImageList;

    TestDICOMLoading();

    ImageList
    LoadFiles( const StringList & files );

    Image::Pointer
    DecorateVerifyCachedImage( const StringList& files, mitk::Image::Pointer cachedImage );

    Image::Pointer
    DecorateVerifyCachedImage( const StringList& files, DICOMTagCache*, mitk::Image::Pointer cachedImage );

    /**
      \brief Dump relevant image information for later comparison.
      \sa CompareImageInformationDumps
    */
    std::string
    DumpImageInformation( const Image* image );

    /**
      \brief Compare two image information dumps.
      \return true, if dumps are sufficiently equal (see parameters)
      \sa DumpImageInformation
    */
    bool
    CompareImageInformationDumps( const std::string& reference,
                                  const std::string& test );

  private:

    typedef std::map<std::string,std::string> KeyValueMap;

    ClassicDICOMSeriesReader::Pointer
    BuildDICOMReader();

    void SetDefaultLocale();

    void ResetUserLocale();

    std::string ComponentTypeToString( int type );

    KeyValueMap ParseDump( const std::string& dump );

    bool CompareSpacedValueFields( const std::string& reference,
                                   const std::string& test,
                                   double eps = mitk::eps );

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

    const char* m_PreviousCLocale;
    std::locale m_PreviousCppLocale;

};

}

#endif

