#ifndef mitkTestDICOMLoading_h
#define mitkTestDICOMLoading_h

#include "mitkDicomSeriesReader.h"

#include "mitkDICOMTestingExports.h"

namespace mitk
{

class mitkDICOMTesting_EXPORT TestDICOMLoading
{
  public:

    typedef DicomSeriesReader::StringContainer StringContainer;
    typedef std::list<DataNode::Pointer> NodeList;
    typedef std::list<Image::Pointer> ImageList;

    TestDICOMLoading();

    ImageList 
    LoadFiles( const StringContainer& files );

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

    void SetDefaultLocale();

    void ResetUserLocale();

    std::string TypeIDToString( const std::type_info& );

    KeyValueMap ParseDump( const std::string& dump );

    bool CompareSpacedValueFields( const std::string& reference, 
                                   const std::string& test,
                                   double eps = mitk::eps );

    const char* m_PreviousCLocale;
    std::locale m_PreviousCppLocale;

};

}

#endif

