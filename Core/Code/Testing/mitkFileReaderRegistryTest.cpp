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

#include "mitkTestingMacros.h"
#include "mitkAbstractFileReader.h"
#include "mitkIFileReader.h"
#include "mitkFileReaderRegistry.h"
#include <mitkBaseData.h>
#include <mitkImage.h>
#include <mitkCustomMimeType.h>

class DummyReader : public mitk::AbstractFileReader
{
public:

  DummyReader(const DummyReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  DummyReader(const std::string& mimeTypeName, const std::string& extension, int priority)
    : mitk::AbstractFileReader()
  {
    mitk::CustomMimeType mimeType(mimeTypeName);
    mimeType.AddExtension(extension);
    mimeType.SetComment("This is a dummy description");

    this->SetMimeType(mimeType);

    this->SetRanking(priority);
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::vector< itk::SmartPointer<mitk::BaseData> >  Read()
  {
    std::vector<mitk::BaseData::Pointer> result;
    return result;
  }

private:

  DummyReader* Clone() const
  {
    return new DummyReader(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
}; // End of internal dummy reader

class DummyReader2 : public mitk::AbstractFileReader
{
public:

  DummyReader2(const DummyReader2& other)
    : mitk::AbstractFileReader(other)
  {
  }

  DummyReader2(const std::string& mimeTypeName, const std::string& extension, int priority)
    : mitk::AbstractFileReader()
  {
    mitk::CustomMimeType mimeType(mimeTypeName);
    mimeType.AddExtension(extension);
    mimeType.SetComment("This is a second dummy description");
    this->SetMimeType(mimeType);

    this->SetRanking(priority);
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader2()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::vector< itk::SmartPointer<mitk::BaseData> >  Read()
  {
    std::vector<mitk::BaseData::Pointer> result;
    return result;
  }

private:

  DummyReader2* Clone() const
  {
    return new DummyReader2(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
}; // End of internal dummy reader 2

/**
*  TODO
*/
int mitkFileReaderRegistryTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("FileReaderRegistry");
  // mitk::FileReaderRegistry::Pointer frm = mitk::FileReaderRegistry::New();
  // MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing FileReaderRegistry instantiation");

  //DummyReader testDR("application/dummy", "test",1);
  //DummyReader otherDR("application/dummy2", "other",1);

  //MITK_TEST_CONDITION_REQUIRED(!testDR.CanRead("/this/is/a/folder/file.tes"),"Negative test of default CanRead() implementation");

  //mitk::FileReaderRegistry* readerRegistry = new mitk::FileReaderRegistry;
  //mitk::IFileReader* returned = readerRegistry->GetReader("bla.test");

  //MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(testDR) != returned,"Testing correct retrieval of FileReader 1/2");

  //returned = readerRegistry->GetReader("other");

  //MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(otherDR) != returned,"Testing correct retrieval of FileReader 2/2");

  //DummyReader mediocreTestDR("application/dummy", "test", 20);
  //DummyReader prettyFlyTestDR("application/dummy", "test", 50);
  //DummyReader2 awesomeTestDR("application/dummy", "test", 100);

  //returned = readerRegistry->GetReader("test");
  //MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyReader2*>(returned), "Testing correct priorized retrieval of FileReader: Best reader");

  // Now to give those readers some options, then we will try again

//  mitk::IFileReader::OptionList options;
//  options.push_back(std::make_pair("isANiceGuy", true));
//  mediocreTestDR.SetOptions(options);
//  options.clear();
//  options.push_back(std::make_pair("canFly", true));
//  prettyFlyTestDR.SetOptions(options);
//  options.push_back(std::make_pair("isAwesome", true));
//  awesomeTestDR.SetOptions(options); //note: awesomeReader canFly and isAwesome

//  // Reset Options, use to define what we want the reader to do
//  options.clear();
//  mitk::IFileReader::OptionNames optionsFilter;
//  optionsFilter.push_back("canFly");
//  returned = readerRegistry->GetReader("test", optionsFilter);
//  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(awesomeTestDR) != returned, "Testing correct retrieval of FileReader with Options: Best reader with options");

//  optionsFilter.push_back("isAwesome");
//  returned = readerRegistry->GetReader("test", optionsFilter);
//  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(awesomeTestDR) != returned, "Testing correct retrieval of FileReader with multiple Options: Best reader with options");

//  optionsFilter.clear();
//  optionsFilter.push_back("isANiceGuy");
//  returned = readerRegistry->GetReader("test", optionsFilter);
//  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(mediocreTestDR) != returned, "Testing correct retrieval of specific FileReader with Options: Low priority reader with specific option");

//  optionsFilter.push_back("canFly");
//  returned = readerRegistry->GetReader("test", optionsFilter);
//  MITK_TEST_CONDITION_REQUIRED(returned == NULL, "Testing correct return of 0 value when no matching reader was found");

//  // Onward to test the retrieval of multiple readers

//  std::vector< mitk::IFileReader* > returnedList;
//  returnedList = readerRegistry->GetReaders("test", optionsFilter);
//  MITK_TEST_CONDITION_REQUIRED(returnedList.empty(), "Testing correct return of zero readers when no matching reader was found, asking for all compatibles");

//  optionsFilter.clear();
//  optionsFilter.push_back("canFly");
//  returnedList = readerRegistry->GetReaders("test", optionsFilter);
//  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 2, "Testing correct return of two readers when two matching reader was found, asking for all compatibles");
//  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyReader2*>(returnedList.front()), "Testing correct priorization of returned Readers with options 1/2");

//  optionsFilter.clear();
//  optionsFilter.push_back("isAwesome");
//  returnedList = readerRegistry->GetReaders("test", optionsFilter);
//  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 1, "Testing correct return of one readers when one matching reader was found, asking for all compatibles");
//  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyReader2*>(returnedList.front()), "Testing correctness of result from former query");

  // And now to verify a working read chain for a mps file:
  //mitk::PointSetReader::Pointer psr = mitk::PointSetReader::New();
  //std::vector<mitk::BaseData::Pointer> basedata;
  //basedata = mitk::FileReaderRegistry::Read("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(basedata.size() > 0, "Testing correct read of PointSet");

  // Testing templated call to ReaderRegistry
  //mitk::PointSet::Pointer pointset = mitk::FileReaderRegistry::Read< mitk::PointSet >("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(pointset.IsNotNull(), "Testing templated call of Read()");

  // And now for something completely different... (Debug)
  // mitk::LegacyFileReaderService::Pointer lfr = mitk::LegacyFileReaderService::New(".nrrd", "Nearly Raw Raster Data");
  //returned = mitk::FileReaderRegistry::GetReader(".nrrd");
  //MITK_TEST_CONDITION_REQUIRED(lfr == returned, "Testing correct retrieval of specific FileReader with Options: Low priority reader with specific option");

  //std::vector<mitk::BaseData::Pointer> image = mitk::FileReaderRegistry::Read("F://Build//MITK-Data//Pic2DplusT.nrrd");
  //MITK_TEST_CONDITION_REQUIRED(image.size() > 0, "Testing whether image was returned or not");
  //mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*> (image.front().GetPointer());
  //MITK_TEST_CONDITION_REQUIRED(image2.IsNotNull(), "Testing if BaseData is an image");

  // Delete this here because it will call the PrototypeServiceFactory::Unget() method
  // of the dummy readers.
  //delete readerRegistry;

  // always end with this!
  MITK_TEST_END();
}
