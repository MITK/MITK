#define MBILOG_ENABLE_DEBUG

#include "mitkTestDICOMLoading.h"

#include <stack>

mitk::TestDICOMLoading::TestDICOMLoading()
{
  MITK_INFO << "TestDICOMLoading()";
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

  StringContainer sortedFiles = DicomSeriesReader::SortSeriesSlices( files );

  DataNode::Pointer node = DicomSeriesReader::LoadDicomSeries( sortedFiles );
 
  ImageList result;

  if (node.IsNotNull())
  {
    Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
  
    result.push_back( image );
  }
  else
  {
  }

  return result;
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
  DumpLine( "Pixeltype",    image->GetPixelType().GetTypeId()->name() );
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

  MITK_DEBUG << "\n----------------------------------------\n" 
            << result.str() 
            << "----------------------------------------\n";

  return result.str();
}

bool
mitk::TestDICOMLoading::CompareSpacedValueFields( const std::string& reference, 
                                                  const std::string& test,
                                                  double eps )
{
  // TODO this should better do a real comparison and tolerate float differences up to 'eps'

  return reference == test;
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
    
    if ( reference.find(refKey) != reference.end() )
    {
      const std::string& testValue = test[refKey];

      testResult &= CompareSpacedValueFields( refValue, testValue );
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
    if (newLinePos == std::string::npos) break;

    std::string line = shredder.substr( 0, newLinePos );
    shredder = shredder.erase( 0, newLinePos+1 );

    MITK_DEBUG << "Line: '" << line << "'";

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

