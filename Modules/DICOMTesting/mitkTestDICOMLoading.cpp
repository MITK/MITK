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



mitk::TestDICOMLoading::ImageList
mitk::TestDICOMLoading
::LoadFiles( const StringList& files )
{
  for (StringList::const_iterator iter = files.begin();
       iter != files.end();
       ++iter)
  {
    MITK_DEBUG << "File " << *iter;
  }

  ImageList result;

  ClassicDICOMSeriesReader::Pointer reader = this->BuildDICOMReader();
  reader->SetInputFiles( files );
  reader->AnalyzeInputFiles();
  reader->PrintOutputs(std::cout,true);
  reader->LoadImages();

  unsigned int numberOfImages = reader->GetNumberOfOutputs();
  for (unsigned imageIndex = 0; imageIndex < numberOfImages; ++imageIndex)
  {
    const DICOMImageBlockDescriptor& block = reader->GetOutput(imageIndex);
    result.push_back( block.GetMitkImage() );
  }

  return result;
}

mitk::ClassicDICOMSeriesReader::Pointer
mitk::TestDICOMLoading
::BuildDICOMReader()
{
  ClassicDICOMSeriesReader::Pointer reader = ClassicDICOMSeriesReader::New();
  reader->SetFixTiltByShearing(true);
  return reader;
}

mitk::Image::Pointer
mitk::TestDICOMLoading
::DecorateVerifyCachedImage( const StringList& files, mitk::DICOMTagCache* tagCache, mitk::Image::Pointer cachedImage )
{
  DICOMImageBlockDescriptor block;

  DICOMImageFrameList framelist;
  for (StringList::const_iterator iter = files.begin();
       iter != files.end();
       ++iter)
  {
    framelist.push_back( DICOMImageFrameInfo::New(*iter) );
  }

  block.SetImageFrameList( framelist );

  block.SetTagCache( tagCache );
  block.SetMitkImage( cachedImage ); // this should/will create a propertylist describing the image slices
  return block.GetMitkImage();
}


mitk::Image::Pointer
mitk::TestDICOMLoading
::DecorateVerifyCachedImage( const StringList& files, mitk::Image::Pointer cachedImage )
{
  ClassicDICOMSeriesReader::Pointer reader = this->BuildDICOMReader();
  reader->SetInputFiles( files );
  reader->AnalyzeInputFiles(); // This just creates a "tag cache and a nice DICOMImageBlockDescriptor.
                               // Both of these could also be produced in a different way. The only
                               // important thing is, that the DICOMImageBlockDescriptor knows a
                               // tag-cache object when PropertyDecorateCachedMitkImageForImageBlockDescriptor
                               // is called.

  if  ( reader->GetNumberOfOutputs() != 1 )
  {
    MITK_ERROR << "Reader produce " << reader->GetNumberOfOutputs() << " images instead of 1 expected..";
    return NULL;
  }

  DICOMImageBlockDescriptor block = reader->GetOutput(0); // creates a block copy
  block.SetMitkImage( cachedImage ); // this should/will create a propertylist describing the image slices
  return block.GetMitkImage();
}

std::string
mitk::TestDICOMLoading::ComponentTypeToString(int type)
{
  if (type == itk::ImageIOBase::UCHAR)
    return "UCHAR";
  else if (type == itk::ImageIOBase::CHAR)
    return "CHAR";
  else if (type == itk::ImageIOBase::USHORT)
    return "USHORT";
  else if (type == itk::ImageIOBase::SHORT)
    return "SHORT";
  else if (type == itk::ImageIOBase::UINT)
    return "UINT";
  else if (type == itk::ImageIOBase::INT)
    return "INT";
  else if (type == itk::ImageIOBase::ULONG)
    return "ULONG";
  else if (type == itk::ImageIOBase::LONG)
    return "LONG";
  else if (type == itk::ImageIOBase::FLOAT)
    return "FLOAT";
  else if (type == itk::ImageIOBase::DOUBLE)
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
  DumpLine( "Pixeltype",    ComponentTypeToString(image->GetPixelType().GetComponentType()) );
  DumpLine( "BitsPerPixel", image->GetPixelType().GetBpe() );
  DumpLine( "Dimension",    image->GetDimension() );

  result << "Dimensions: ";
  for (unsigned int dim = 0; dim < image->GetDimension(); ++dim)
    result << image->GetDimension(dim) << " ";
  result << "\n";

  // geometry data
  result << "Geometry: \n";
  const TimeGeometry* timeGeometry = image->GetTimeGeometry();
  BaseGeometry* geometry = timeGeometry->GetGeometryForTimeStep(0);
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
      const TimeBounds timeBounds = timeGeometry->GetTimeBounds();
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
      if ( this->StringToNumber(testToken, testNumber) )
      {
        // print-out compared tokens if DEBUG output allowed
        MITK_DEBUG << "Reference Token '" << refToken << "'" << " value " << refNumber
                   << ", test Token '" << testToken << "'" << " value " << testNumber;

        bool old_result = result;

        result &= ( std::abs(refNumber - testNumber) < 0.0001f /*mitk::eps*/ );
        // log the token/number which causes the test to fail
        if( old_result != result)
        {
          MITK_ERROR << std::setprecision(16) << "Reference Token '" << refToken << "'" << " value " << refNumber
                     << ", test Token '" << testToken << "'" << " value " << testNumber;

          MITK_ERROR << "[FALSE] - difference: " << std::setprecision(16) <<  std::abs(refNumber - testNumber) << " EPS: " << 0.0001f; //mitk::eps;
        }
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

