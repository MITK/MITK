//#define MBILOG_ENABLE_DEBUG

#include "mitkTestDICOMLoading.h"

#include <stack>

mitk::TestDICOMLoading::TestDICOMLoading()
:m_PreviousCLocale(NULL)
{
}

void mitk::TestDICOMLoading::SetDefaultLocale()
{
  // remember old locale only once
  if (m_PreviousCLocale == NULL)
  {
    m_PreviousCLocale = setlocale(LC_NUMERIC, NULL);

    // set to "C"
    setlocale(LC_NUMERIC, "C");
  
    m_PreviousCppLocale = std::cin.getloc();
    
    std::locale l( "C" );
    std::cin.imbue(l);
    std::cout.imbue(l);
  }
}

void mitk::TestDICOMLoading::ResetUserLocale()
{
  if (m_PreviousCLocale)
  {
    setlocale(LC_NUMERIC, m_PreviousCLocale);
  
    std::cin.imbue(m_PreviousCppLocale);
    std::cout.imbue(m_PreviousCppLocale);
    
    m_PreviousCLocale = NULL;
  }
}



mitk::TestDICOMLoading::ImageList mitk::TestDICOMLoading::LoadFiles( const StringContainer& files )
{
  for (StringContainer::const_iterator iter = files.begin();
       iter != files.end();
       ++iter)
  {
    MITK_DEBUG << "File " << *iter;
  }
   
  ImageList result;

  DicomSeriesReader::UidFileNamesMap seriesInFiles = DicomSeriesReader::GetSeries( files );

  // TODO sort series UIDs, implementation of map iterator might differ on different platforms (or verify this is a standard topic??)
  for (DicomSeriesReader::UidFileNamesMap::const_iterator seriesIter = seriesInFiles.begin();
       seriesIter != seriesInFiles.end();
       ++seriesIter)
  {
    StringContainer files = seriesIter->second;

    DataNode::Pointer node = DicomSeriesReader::LoadDicomSeries( files );

    if (node.IsNotNull())
    {
      Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
    
      result.push_back( image );
    }
    else
    {
    }
  }

  return result;
}

std::string
mitk::TestDICOMLoading::TypeIDToString(const std::type_info& ti)
{
  if (ti == typeid(unsigned char))
    return "UCHAR";
  else if (ti == typeid(char))
    return "CHAR";
  else if (ti == typeid(unsigned short))
    return "USHORT";
  else if (ti == typeid(short))
    return "SHORT";
  else if (ti == typeid(unsigned int))
    return "UINT";
  else if (ti == typeid(int))
    return "INT";
  else if (ti == typeid(long unsigned int))
    return "ULONG";
  else if (ti == typeid(long int))
    return "LONG";
  else if (ti == typeid(float))
    return "FLOAT";
  else if (ti == typeid(double))
    return "DOUBLE";
  else
    return "UNKNOWN";
}


// add a line to stringstream result (see DumpImageInformation
#define DumpLine(field, data) DumpILine(0, field, data)

// add an indented(!) line to stringstream result (see DumpImageInformation
#define DumpILine(indent, field, data) \
{ \
  std::string DumpLine_INDENT; DumpLine_INDENT.resize(indent, ' ' ); \
  result << DumpLine_INDENT << field << ": " << data << "\n"; \
}

std::string
mitk::TestDICOMLoading::DumpImageInformation( const Image* image )
{
  std::stringstream result;

  if (image == NULL) return result.str();

  SetDefaultLocale();

  // basic image data
  DumpLine( "Pixeltype",    TypeIDToString( (image->GetPixelType().GetTypeId()) ));
  DumpLine( "BitsPerPixel", image->GetPixelType().GetBpe() );
  DumpLine( "Dimension",    image->GetDimension() );

  result << "Dimensions: ";
  for (unsigned int dim = 0; dim < image->GetDimension(); ++dim)
    result << image->GetDimension(dim) << " ";
  result << "\n";

  // geometry data
  result << "Geometry: \n";
  Geometry3D* geometry = image->GetGeometry();
  if (geometry)
  {
    AffineTransform3D* transform = geometry->GetIndexToWorldTransform();
    if (transform)
    {
      result << "  " << "Matrix: ";
      const AffineTransform3D::MatrixType& matrix = transform->GetMatrix();
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
          result << matrix[i][j] << " ";
      result << "\n";

      result << "  " << "Offset: ";
      const AffineTransform3D::OutputVectorType& offset = transform->GetOffset();
      for (unsigned int i = 0; i < 3; ++i)
          result << offset[i] << " ";
      result << "\n";

      result << "  " << "Center: ";
      const AffineTransform3D::InputPointType& center = transform->GetCenter();
      for (unsigned int i = 0; i < 3; ++i)
          result << center[i] << " ";
      result << "\n";

      result << "  " << "Translation: ";
      const AffineTransform3D::OutputVectorType& translation = transform->GetTranslation();
      for (unsigned int i = 0; i < 3; ++i)
          result << translation[i] << " ";
      result << "\n";

      result << "  " << "Scale: ";
      const double* scale = transform->GetScale();
      for (unsigned int i = 0; i < 3; ++i)
          result << scale[i] << " ";
      result << "\n";

      result << "  " << "Origin: ";
      const Point3D& origin = geometry->GetOrigin();
      for (unsigned int i = 0; i < 3; ++i)
          result << origin[i] << " ";
      result << "\n";

      result << "  " << "Spacing: ";
      const Vector3D& spacing = geometry->GetSpacing();
      for (unsigned int i = 0; i < 3; ++i)
          result << spacing[i] << " ";
      result << "\n";

      result << "  " << "TimeBounds: ";
      const TimeBounds timeBounds = geometry->GetTimeBounds();
      for (unsigned int i = 0; i < 2; ++i)
          result << timeBounds[i] << " ";
      result << "\n";


    }
  }
  
  ResetUserLocale();

  return result.str();
}

std::string 
mitk::TestDICOMLoading::trim(const std::string& pString, 
                             const std::string& pWhitespace)
{
  const size_t beginStr = pString.find_first_not_of(pWhitespace);
  if (beginStr == std::string::npos)
  {
    // no content
    return "";
  }

  const size_t endStr = pString.find_last_not_of(pWhitespace);
  const size_t range = endStr - beginStr + 1;

  return pString.substr(beginStr, range);
}

std::string 
mitk::TestDICOMLoading::reduce(const std::string& pString, 
                               const std::string& pFill,
                               const std::string& pWhitespace)
{
  // trim first
  std::string result(trim(pString, pWhitespace));

  // replace sub ranges
  size_t beginSpace = result.find_first_of(pWhitespace);
  while (beginSpace != std::string::npos)
  {
    const size_t endSpace =
      result.find_first_not_of(pWhitespace, beginSpace);
    const size_t range = endSpace - beginSpace;

    result.replace(beginSpace, range, pFill);

    const size_t newStart = beginSpace + pFill.length();
    beginSpace = result.find_first_of(pWhitespace, newStart);
  }

  return result;
}


bool
mitk::TestDICOMLoading::CompareSpacedValueFields( const std::string& reference, 
                                                  const std::string& test,
                                                  double /*eps*/ )
{
  bool result(true);

  // tokenize string, compare each token, if possible by float comparison
  std::stringstream referenceStream(reduce(reference));
  std::stringstream testStream(reduce(test));

  std::string refToken;
  std::string testToken;
  while ( std::getline( referenceStream,  refToken, ' ' ) && 
          std::getline (     testStream, testToken, ' ' ) )
  {
    float refNumber;
    float testNumber;
    if ( this->StringToNumber(refToken, refNumber) )
    {
      MITK_DEBUG << "Reference Token '" << refToken << "'" << " value " << refNumber
                 << ", test Token '" << refToken << "'" << " value " << refNumber;
      if ( this->StringToNumber(testToken, testNumber) )
      {
        result &= ( fabs(refNumber - testNumber) < mitk::eps );
      }
      else
      {
        MITK_ERROR << refNumber << " cannot be compared to '" << testToken << "'";
      }
    }
    else
    {
      MITK_DEBUG << "Token '" << refToken << "'" << " handled as string";
      result &= refToken == testToken;
    }
  }

  if ( std::getline( referenceStream, refToken, ' ' ) )
  {
    MITK_ERROR << "Reference string still had values when test string was already parsed: ref '" << reference << "', test '" << test << "'";
    result = false;
  }
  else if ( std::getline( testStream, testToken, ' ' ) )
  {
    MITK_ERROR << "Test string still had values when reference string was already parsed: ref '" << reference << "', test '" << test << "'";
    result = false;
  }

  return result;
}

bool
mitk::TestDICOMLoading::CompareImageInformationDumps( const std::string& referenceDump,
                                                      const std::string& testDump )
{
  KeyValueMap reference = ParseDump(referenceDump);
  KeyValueMap test = ParseDump(testDump);

  bool testResult(true);

  // verify all expected values
  for (KeyValueMap::const_iterator refIter = reference.begin(); 
       refIter != reference.end(); 
       ++refIter)
  {
    const std::string& refKey = refIter->first;
    const std::string& refValue = refIter->second;
    
    if ( test.find(refKey) != test.end() )
    {
      const std::string& testValue = test[refKey];

      bool thisTestResult = CompareSpacedValueFields( refValue, testValue );
      testResult &= thisTestResult;
      
      MITK_DEBUG << refKey << ": '" << refValue << "' == '" << testValue << "' ? " << (thisTestResult?"YES":"NO");
    }
    else
    {
      MITK_ERROR << "Reference dump contains a key'" << refKey << "' (value '" << refValue << "')." ;
      MITK_ERROR << "This key is expected to be generated for tests (but was not). Most probably you need to update your test data.";
      return false;
    }
  }

  // now check test dump does not contain any additional keys
  for (KeyValueMap::const_iterator testIter = test.begin(); 
       testIter != test.end(); 
       ++testIter)
  {
    const std::string& key = testIter->first;
    const std::string& value = testIter->second;

    if ( reference.find(key) == reference.end() )
    {
      MITK_ERROR << "Test dump contains an unexpected key'" << key << "' (value '" << value << "')." ;
      MITK_ERROR << "This key is not expected. Most probably you need to update your test data.";
      return false;
    }
  }

  return testResult;
}

mitk::TestDICOMLoading::KeyValueMap
mitk::TestDICOMLoading::ParseDump( const std::string& dump )
{
  KeyValueMap parsedResult;

  std::string shredder(dump);

  std::stack<std::string> surroundingKeys;

  std::stack<std::string::size_type> expectedIndents;
  expectedIndents.push(0);

  while (true)
  {
    std::string::size_type newLinePos = shredder.find( '\n' );
    if (newLinePos == std::string::npos || newLinePos == 0) break;

    std::string line = shredder.substr( 0, newLinePos );
    shredder = shredder.erase( 0, newLinePos+1 );

    std::string::size_type keyPosition = line.find_first_not_of( ' ' );
    std::string::size_type colonPosition = line.find( ':' );
    
    std::string key = line.substr(keyPosition, colonPosition - keyPosition);
    std::string::size_type firstSpacePosition = key.find_first_of(" ");
    if (firstSpacePosition != std::string::npos)
    {
      key.erase(firstSpacePosition);
    }

    if ( keyPosition > expectedIndents.top() )
    {
      // more indent than before
      expectedIndents.push(keyPosition);
    }
    else if (keyPosition == expectedIndents.top() )
    {
      if (!surroundingKeys.empty())
      {
        surroundingKeys.pop(); // last of same length
      }
    }
    else
    {
      // less indent than before
      do expectedIndents.pop();
      while (expectedIndents.top() != keyPosition); // unwind until current indent is found
    }

    if (!surroundingKeys.empty())
    {
      key = surroundingKeys.top() + "." + key; // construct current key name
    }

    surroundingKeys.push(key); // this is the new embracing key

    std::string value = line.substr(colonPosition+1);
    
    MITK_DEBUG << "  Key: '" << key << "' value '" << value << "'" ;

    parsedResult[key] = value; // store parsing result
  }

  return parsedResult;
}

