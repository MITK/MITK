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
#include <mitkBaseDataIOFactory.h>
#include <mitkLegacyFileReaderService.h>
#include <mitkImage.h>

#include <mitkPointSetReader.h>
#include <mitkPointSet.h>
#include <mitkCoreObjectFactory.h>

class DummyReader : public mitk::AbstractFileReader
{
public:

  DummyReader(const DummyReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  DummyReader(const std::string& extension, int priority)
    : mitk::AbstractFileReader(extension, "This is a dummy description")
  {
    m_Priority = priority;
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::list< itk::SmartPointer<mitk::BaseData> >  Read(const std::istream& /*stream*/, mitk::DataStorage* /*ds*/ = 0)
  {
    std::list<mitk::BaseData::Pointer> result;
    return result;
  }

  virtual void SetOptions(const std::list< mitk::FileServiceOption >& options )
  {
    m_Options = options;
    //m_Registration.SetProperties(ConstructServiceProperties());
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

  DummyReader2(const std::string& extension, int priority)
    : mitk::AbstractFileReader(extension, "This is a second dummy description")
  {
    m_Priority = priority;
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader2()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::list< itk::SmartPointer<mitk::BaseData> >  Read(const std::istream& /*stream*/, mitk::DataStorage* /*ds*/ = 0)
  {
    std::list<mitk::BaseData::Pointer> result;
    return result;
  }

  virtual void SetOptions(const std::list< mitk::FileServiceOption >& options )
  {
    m_Options = options;
    //m_Registration.SetProperties(ConstructServiceProperties());
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
int mitkFileReaderManagerTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("FileReaderManager");
  // mitk::FileReaderRegistry::Pointer frm = mitk::FileReaderRegistry::New();
  // MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing FileReaderRegistry instantiation");

  DummyReader testDR("test",1);
  DummyReader otherDR("other",1);

  MITK_TEST_CONDITION_REQUIRED(testDR.CanRead("/this/is/a/folder/file.test"),"Positive test of default CanRead() implementation");
  MITK_TEST_CONDITION_REQUIRED(!testDR.CanRead("/this/is/a/folder/file.tes"),"Negative test of default CanRead() implementation");

  mitk::FileReaderRegistry* readerManager = new mitk::FileReaderRegistry;
  mitk::IFileReader* returned = readerManager->GetReader("test");

  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(testDR) != returned,"Testing correct retrieval of FileReader 1/2");

  returned = readerManager->GetReader("other");

  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(otherDR) != returned,"Testing correct retrieval of FileReader 2/2");

  DummyReader mediocreTestDR("test", 20);
  DummyReader prettyFlyTestDR("test", 50);
  DummyReader2 awesomeTestDR("test", 100);

  returned = readerManager->GetReader("test");
  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyReader2*>(returned), "Testing correct priorized retrieval of FileReader: Best reader");

  // Now to give those readers some options, then we will try again

  std::list< mitk::FileServiceOption > options;
  options.push_front(std::make_pair("isANiceGuy", true));
  mediocreTestDR.SetOptions(options);
  options.clear();
  options.push_front(std::make_pair("canFly", true));
  prettyFlyTestDR.SetOptions(options);
  options.push_front(std::make_pair("isAwesome", true));
  awesomeTestDR.SetOptions(options); //note: awesomeReader canFly and isAwesome

  // Reset Options, use to define what we want the reader to do
  options.clear();
  std::list<std::string> optionsFilter;
  optionsFilter.push_front("canFly");
  returned = readerManager->GetReader("test", optionsFilter);
  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(awesomeTestDR) != returned, "Testing correct retrieval of FileReader with Options: Best reader with options");

  optionsFilter.push_front("isAwesome");
  returned = readerManager->GetReader("test", optionsFilter);
  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(awesomeTestDR) != returned, "Testing correct retrieval of FileReader with multiple Options: Best reader with options");

  optionsFilter.clear();
  optionsFilter.push_front("isANiceGuy");
  returned = readerManager->GetReader("test", optionsFilter);
  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileReader&>(mediocreTestDR) != returned, "Testing correct retrieval of specific FileReader with Options: Low priority reader with specific option");

  optionsFilter.push_front("canFly");
  returned = readerManager->GetReader("test", optionsFilter);
  MITK_TEST_CONDITION_REQUIRED(returned == NULL, "Testing correct return of 0 value when no matching reader was found");

  // Onward to test the retrieval of multiple readers

  std::vector< mitk::IFileReader* > returnedList;
  returnedList = readerManager->GetReaders("test", optionsFilter);
  MITK_TEST_CONDITION_REQUIRED(returnedList.empty(), "Testing correct return of zero readers when no matching reader was found, asking for all compatibles");

  optionsFilter.clear();
  optionsFilter.push_back("canFly");
  returnedList = readerManager->GetReaders("test", optionsFilter);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 2, "Testing correct return of two readers when two matching reader was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyReader2*>(returnedList.front()), "Testing correct priorization of returned Readers with options 1/2");

  optionsFilter.clear();
  optionsFilter.push_back("isAwesome");
  returnedList = readerManager->GetReaders("test", optionsFilter);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 1, "Testing correct return of one readers when one matching reader was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyReader2*>(returnedList.front()), "Testing correctness of result from former query");

  // And now to verify a working read chain for a mps file:
  //mitk::PointSetReader::Pointer psr = mitk::PointSetReader::New();
  //std::list<mitk::BaseData::Pointer> basedata;
  //basedata = mitk::FileReaderRegistry::Read("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(basedata.size() > 0, "Testing correct read of PointSet");

  // Need to instanciate the CoreObjectFactory, so legacy Readers are available
  mitk::CoreObjectFactory::GetInstance();

  // Testing templated call to ReaderManager
  //mitk::PointSet::Pointer pointset = mitk::FileReaderRegistry::Read< mitk::PointSet >("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(pointset.IsNotNull(), "Testing templated call of Read()");

  // And now for something completely different... (Debug)
  // mitk::LegacyFileReaderService::Pointer lfr = mitk::LegacyFileReaderService::New(".nrrd", "Nearly Raw Raster Data");
  //returned = mitk::FileReaderRegistry::GetReader(".nrrd");
  //MITK_TEST_CONDITION_REQUIRED(lfr == returned, "Testing correct retrieval of specific FileReader with Options: Low priority reader with specific option");

  //std::list<mitk::BaseData::Pointer> image = mitk::FileReaderRegistry::Read("F://Build//MITK-Data//Pic2DplusT.nrrd");
  //MITK_TEST_CONDITION_REQUIRED(image.size() > 0, "Testing whether image was returned or not");
  //mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*> (image.front().GetPointer());
  //MITK_TEST_CONDITION_REQUIRED(image2.IsNotNull(), "Testing if BaseData is an image");

  // Delete this here because it will call the PrototypeServiceFactory::Unget() method
  // of the dummy readers.
  delete readerManager;

  // always end with this!
  MITK_TEST_END();
}
