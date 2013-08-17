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

#include <mitkTestingMacros.h>
#include <mitkAbstractFileWriter.h>
#include <mitkIFileWriter.h>
#include <mitkFileWriterManager.h>
#include <mitkFileReaderManager.h>
#include <mitkBaseData.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkLegacyFileWriterService.h>
#include <mitkImage.h>
#include <mitkCoreObjectFactory.h>
#include <mitkPointSetWriter.h>
#include <mitkPointSet.h>


class DummyWriter : public mitk::AbstractFileWriter
{

public:

  using AbstractFileWriter::Write;

  virtual void Write(const mitk::BaseData* /*data*/, std::ostream& /*stream*/ )
  {  }

  virtual void SetOptions(const std::list< std::string >& options )
  {
    m_Options = options;
    m_Registration.SetProperties(ConstructServiceProperties());
  }

  DummyWriter(const std::string& basedataType, const std::string& extension, int priority)
    : mitk::AbstractFileWriter(basedataType, extension, "This is a dummy description")
  {
    m_Priority = priority;
  }

}; // End of internal dummy Writer

/**
 *  TODO
 */
int mitkFileWriterManagerTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("FileWriterManager");
 // mitk::FileWriterManager::Pointer frm = mitk::FileWriterManager::New();
 // MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing FileWriterManager instantiation");

  DummyWriter testDR("testdata", "test", 1);
  DummyWriter otherDR("testdata", "other", 1);

 // MITK_TEST_CONDITION_REQUIRED(testDR->CanWrite("/this/is/a/folder/file.test"),"Positive test of default CanRead() implementation");
 // MITK_TEST_CONDITION_REQUIRED(!testDR->CanWrite("/this/is/a/folder/file.tes"),"Negative test of default CanRead() implementation");

  mitk::IFileWriter* returned = mitk::FileWriterManager::GetWriter("test");

  MITK_TEST_CONDITION_REQUIRED(&static_cast<mitk::IFileWriter&>(testDR) == returned,"Testing correct retrieval of FileWriter 1/2");

  returned = mitk::FileWriterManager::GetWriter("other");

  MITK_TEST_CONDITION_REQUIRED(&static_cast<mitk::IFileWriter&>(otherDR) == returned,"Testing correct retrieval of FileWriter 2/2");

  DummyWriter mediocreTestDR("testdata", "test", 20);
  DummyWriter prettyFlyTestDR("testdata", "test", 50);
  DummyWriter awesomeTestDR("testdata", "test", 100);

  returned = mitk::FileWriterManager::GetWriter("test");
  MITK_TEST_CONDITION_REQUIRED(&static_cast<mitk::IFileWriter&>(awesomeTestDR) == returned, "Testing correct priorized retrieval of FileWriter: Best Writer");

  // Now to give those Writers some options, then we will try again
  std::list<std::string> options;
  options.push_front("isANiceGuy");
  mediocreTestDR.SetOptions(options);
  options.clear();
  options.push_front("canFly");
  prettyFlyTestDR.SetOptions(options);
  options.push_front("isAwesome");
  awesomeTestDR.SetOptions(options); //note: awesomeWriter canFly and isAwesome

  // Reset Options, use to define what we want the Writer to do
  options.clear();
  options.push_front("canFly");
  returned = mitk::FileWriterManager::GetWriter("test", options);
  MITK_TEST_CONDITION_REQUIRED(&static_cast<mitk::IFileWriter&>(awesomeTestDR) == returned, "Testing correct retrieval of FileWriter with Options: Best Writer with options");

  options.push_front("isAwesome");
  returned = mitk::FileWriterManager::GetWriter("test", options);
  MITK_TEST_CONDITION_REQUIRED(&static_cast<mitk::IFileWriter&>(awesomeTestDR) == returned, "Testing correct retrieval of FileWriter with multiple Options: Best Writer with options");

  options.clear();
  options.push_front("isANiceGuy");
  returned = mitk::FileWriterManager::GetWriter("test", options);
  MITK_TEST_CONDITION_REQUIRED(&static_cast<mitk::IFileWriter&>(mediocreTestDR) == returned, "Testing correct retrieval of specific FileWriter with Options: Low priority Writer with specific option");

  options.push_front("canFly");
  returned = mitk::FileWriterManager::GetWriter("test", options);
  MITK_TEST_CONDITION_REQUIRED(returned == 0, "Testing correct return of 0 value when no matching Writer was found");

  // Onward to test the retrieval of multiple Writers

  std::vector< mitk::IFileWriter* > returnedList;
  returnedList = mitk::FileWriterManager::GetWriters("test", options);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 0, "Testing correct return of zero Writers when no matching Writer was found, asking for all compatibles");

  options.clear();
  options.push_back("canFly");
  returnedList = mitk::FileWriterManager::GetWriters("test", options);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 2, "Testing correct return of two Writers when two matching Writer was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(returnedList.front() == &static_cast<mitk::IFileWriter&>(awesomeTestDR), "Testing correct priorization of returned Writers with options 1/2");
  MITK_TEST_CONDITION_REQUIRED(returnedList.back() == &static_cast<mitk::IFileWriter&>(prettyFlyTestDR), "Testing correct priorization of returned Writers with options 2/2");

  options.clear();
  options.push_back("isAwesome");
  returnedList = mitk::FileWriterManager::GetWriters("test", options);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 1, "Testing correct return of one Writers when one matching Writer was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(returnedList.front() == &static_cast<mitk::IFileWriter&>(awesomeTestDR), "Testing correctness of result from former query");

  mitk::CoreObjectFactory::GetInstance();
  mitk::Image::Pointer image = mitk::FileReaderManager::Read<mitk::Image>("F://Build//MITK-Data//Pic2DplusT.nrrd");

  mitk::FileWriterManager::Write(image.GetPointer(), "F://Build//MITK-Data//Pic2DplusTcopy.nrrd");

  //// And now to verify a working read chain for a mps file:
  //mitk::PointSetWriter::Pointer psr = mitk::PointSetWriter::New();
  //mitk::BaseData::Pointer basedata;
  //basedata = mitk::FileWriterManager::Read("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(basedata.IsNotNull(), "Testing correct read of PointSet");

  //// Testing templated call to WriterManager
  //mitk::PointSet::Pointer pointset = mitk::FileWriterManager::Read< mitk::PointSet >("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(pointset.IsNotNull(), "Testing templated call of Read()");

  //// And now for something completely different... (Debug)
  //mitk::LegacyFileWriterService::Pointer lfr = mitk::LegacyFileWriterService::New(".nrrd", "Nearly Raw Raster Data");
  //returned = mitk::FileWriterManager::GetWriter(".nrrd");
  //MITK_TEST_CONDITION_REQUIRED(lfr == returned, "Testing correct retrieval of specific FileWriter with Options: Low priority Writer with specific option");

  //mitk::BaseData::Pointer image = mitk::FileWriterManager::Read("F://Build//MITK-Data//Pic2DplusT.nrrd");
  //MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(), "Testing whether BaseData is empty or not");
  //mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*> (image.GetPointer());
  //MITK_TEST_CONDITION_REQUIRED(image2.IsNotNull(), "Testing if BaseData is an image");

  //// always end with this!
  MITK_TEST_END();
}
