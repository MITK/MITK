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
#include <mitkFileWriterRegistry.h>
#include <mitkFileReaderRegistry.h>
#include <mitkBaseData.h>
#include <mitkIOUtil.h>

class DummyBaseData : public mitk::BaseData
{
public:

  mitkClassMacro(DummyBaseData, mitk::BaseData)
  itkNewMacro(Self)

  void SetRequestedRegion(const itk::DataObject * /*data*/) {}
  void SetRequestedRegionToLargestPossibleRegion() {}
  bool RequestedRegionIsOutsideOfTheBufferedRegion() { return false; }
  bool VerifyRequestedRegion() { return true; }
};

class DummyWriter : public mitk::AbstractFileWriter
{

public:

  DummyWriter(const DummyWriter& other)
    : mitk::AbstractFileWriter(other)
    , m_Content("Hi there stream")
  {
  }

  DummyWriter(const std::string& basedataType, const std::string& extension, int ranking)
    : mitk::AbstractFileWriter(basedataType, extension, "This is a dummy description")
    , m_Content("Hi there stream")
  {
    this->SetRanking(ranking);
    m_ServiceReg = this->RegisterService();
  }

  ~DummyWriter()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using AbstractFileWriter::Write;

  virtual void Write(const mitk::BaseData* data, std::ostream& stream)
  {
    MITK_TEST_CONDITION_REQUIRED(dynamic_cast<const DummyBaseData*>(data), "Correct data type")
    stream << m_Content;
  }

  std::string m_Content;

private:

  DummyWriter* Clone() const
  {
    return new DummyWriter(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;

}; // End of internal dummy Writer

class DummyWriter2 : public mitk::AbstractFileWriter
{

public:

  DummyWriter2(const DummyWriter2& other)
    : mitk::AbstractFileWriter(other)
    , m_Content("hi there file path")
  {
  }

  DummyWriter2(const std::string& basedataType, const std::string& extension, int ranking)
    : mitk::AbstractFileWriter(basedataType, extension, "This is a dummy description")
    , m_Content("hi there file path")
  {
    this->SetRanking(ranking);
    m_ServiceReg = this->RegisterService();
  }

  ~DummyWriter2()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using AbstractFileWriter::Write;

  virtual void Write(const mitk::BaseData* data, const std::string& filePath )
  {
    MITK_TEST_CONDITION_REQUIRED(dynamic_cast<const DummyBaseData*>(data), "Correct data type")
    std::ofstream fileStream(filePath.c_str());
    fileStream << m_Content;
  }

  virtual void Write(const mitk::BaseData* data, std::ostream& stream )
  {
    mitk::AbstractFileWriter::Write(data, stream);
  }

  virtual bool CanWrite(const mitk::BaseData *data) const
  {
    return dynamic_cast<const DummyBaseData*>(data);
  }

  std::string m_Content;

private:

  DummyWriter2* Clone() const
  {
    return new DummyWriter2(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;

}; // End of internal dummy Writer 2


void TestStreamMethods()
{
  DummyWriter dummyWriter(DummyBaseData::GetStaticNameOfClass(), "stream", 100);
  DummyWriter2 dummyWriter2(DummyBaseData::GetStaticNameOfClass(), "file", 50);
  mitk::FileWriterRegistry writerRegistry;

  // Test DummyWriter, which always uses a ostream for writing, even
  // when a file path is used
  DummyBaseData dummyData;
  std::stringstream oss;
  writerRegistry.Write(&dummyData, oss);
  MITK_TEST_CONDITION_REQUIRED(dummyWriter.m_Content == oss.str(), "Dummy stream writer")

  std::string content;
  {
    std::ofstream tmpStream;
    std::string tmpFileName = mitk::IOUtil::CreateTemporaryFile(tmpStream);
    writerRegistry.Write(&dummyData, tmpFileName);

    std::ifstream tmpInput(tmpFileName.c_str());
    std::getline(tmpInput, content);
    tmpInput.close();
    tmpStream.close();
    std::remove(tmpFileName.c_str());
  }
  MITK_TEST_CONDITION_REQUIRED(dummyWriter.m_Content == content, "Dummy stream writer")

  // Test DummyWriter2, which always uses a real file for writing, even
  // when a std::ostream object is given
  std::stringstream oss2;
  dummyWriter2.Write(&dummyData, oss2);
  MITK_TEST_CONDITION_REQUIRED(dummyWriter2.m_Content == oss2.str(), "Dummy 2 stream writer")

  std::string content2;
  {
    std::ofstream tmpStream;
    std::string tmpFileName = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.file");
    writerRegistry.Write(&dummyData, tmpFileName);

    std::ifstream tmpInput(tmpFileName.c_str());
    std::getline(tmpInput, content2);
    tmpInput.close();
    std::remove(tmpFileName.c_str());
  }
  MITK_TEST_CONDITION_REQUIRED(dummyWriter2.m_Content == content2, "Dummy 2 stream writer")
}

/**
 *  TODO
 */
int mitkFileWriterRegistryTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("FileWriterRegistry");

  TestStreamMethods();


 // mitk::FileWriterRegistry::Pointer frm = mitk::FileWriterRegistry::New();
 // MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing FileWriterRegistry instantiation");

  DummyWriter testDR("testdata", "test", 1);
  DummyWriter otherDR("testdata", "other", 1);

 // MITK_TEST_CONDITION_REQUIRED(testDR->CanWrite("/this/is/a/folder/file.test"),"Positive test of default CanRead() implementation");
 // MITK_TEST_CONDITION_REQUIRED(!testDR->CanWrite("/this/is/a/folder/file.tes"),"Negative test of default CanRead() implementation");

  mitk::FileWriterRegistry* writerRegistry = new mitk::FileWriterRegistry;
  mitk::IFileWriter* returned = writerRegistry->GetWriter("", "test");

  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileWriter&>(testDR) != returned,"Testing correct retrieval of FileWriter 1/2");

  returned = writerRegistry->GetWriter("", "other");

  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileWriter&>(otherDR) != returned,"Testing correct retrieval of FileWriter 2/2");

  DummyWriter mediocreTestDR("testdata", "test", 20);
  DummyWriter prettyFlyTestDR("testdata", "test", 50);
  DummyWriter2 awesomeTestDR("testdata", "test", 100);

  returned = writerRegistry->GetWriter("test");
  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyWriter2*>(returned), "Testing correct priorized retrieval of FileWriter: Best Writer");

  // Now to give those Writers some options, then we will try again
  mitk::IFileWriter::OptionList options;
  options.push_back(std::make_pair("isANiceGuy", true));
  mediocreTestDR.SetOptions(options);
  options.clear();
  options.push_back(std::make_pair("canFly", true));
  prettyFlyTestDR.SetOptions(options);
  options.push_back(std::make_pair("isAwesome", true));
  awesomeTestDR.SetOptions(options); //note: awesomeWriter canFly and isAwesome

  // Reset Options, use to define what we want the Writer to do
  mitk::IFileWriter::OptionNames optionFilter;
  optionFilter.push_back("canFly");
  returned = writerRegistry->GetWriter("", "test", optionFilter);
  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileWriter&>(awesomeTestDR) != returned, "Testing correct retrieval of FileWriter with Options: Best Writer with options");

  optionFilter.push_back("isAwesome");
  returned = writerRegistry->GetWriter("", "test", optionFilter);
  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileWriter&>(awesomeTestDR) != returned, "Testing correct retrieval of FileWriter with multiple Options: Best Writer with options");

  optionFilter.clear();
  optionFilter.push_back("isANiceGuy");
  returned = writerRegistry->GetWriter("", "test", optionFilter);
  MITK_TEST_CONDITION_REQUIRED(returned && &static_cast<mitk::IFileWriter&>(mediocreTestDR) != returned, "Testing correct retrieval of specific FileWriter with Options: Low priority Writer with specific option");

  optionFilter.push_back("canFly");
  returned = writerRegistry->GetWriter("", "test", optionFilter);
  MITK_TEST_CONDITION_REQUIRED(returned == NULL, "Testing correct return of 0 value when no matching Writer was found");

  // Onward to test the retrieval of multiple Writers

  std::vector< mitk::IFileWriter* > returnedList;
  returnedList = writerRegistry->GetWriters("", "test", optionFilter);
  MITK_TEST_CONDITION_REQUIRED(returnedList.empty(), "Testing correct return of zero Writers when no matching Writer was found, asking for all compatibles");

  optionFilter.clear();
  optionFilter.push_back("canFly");
  returnedList = writerRegistry->GetWriters("", "test", optionFilter);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 2, "Testing correct return of two Writers when two matching Writer was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyWriter2*>(returnedList.front()), "Testing correct priorization of returned Writers with options 1/2");

  optionFilter.clear();
  optionFilter.push_back("isAwesome");
  returnedList = writerRegistry->GetWriters("", "test", optionFilter);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 1, "Testing correct return of one Writers when one matching Writer was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(dynamic_cast<DummyWriter2*>(returnedList.front()), "Testing correctness of result from former query");

  //mitk::CoreObjectFactory::GetInstance();
  //mitk::FileReaderRegistry readerRegistry;
  //mitk::Image::Pointer image = readerRegistry.Read<mitk::Image>("F://Build//MITK-Data//Pic2DplusT.nrrd");

  //writerRegistry->Write(image.GetPointer(), "F://Build//MITK-Data//Pic2DplusTcopy.nrrd");

  //// And now to verify a working read chain for a mps file:
  //mitk::PointSetWriter::Pointer psr = mitk::PointSetWriter::New();
  //mitk::BaseData::Pointer basedata;
  //basedata = mitk::FileWriterRegistry::Read("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(basedata.IsNotNull(), "Testing correct read of PointSet");

  //// Testing templated call to WriterRegistry
  //mitk::PointSet::Pointer pointset = mitk::FileWriterRegistry::Read< mitk::PointSet >("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(pointset.IsNotNull(), "Testing templated call of Read()");

  //// And now for something completely different... (Debug)
  //mitk::LegacyFileWriterService::Pointer lfr = mitk::LegacyFileWriterService::New(".nrrd", "Nearly Raw Raster Data");
  //returned = mitk::FileWriterRegistry::GetWriter(".nrrd");
  //MITK_TEST_CONDITION_REQUIRED(lfr == returned, "Testing correct retrieval of specific FileWriter with Options: Low priority Writer with specific option");

  //mitk::BaseData::Pointer image = mitk::FileWriterRegistry::Read("F://Build//MITK-Data//Pic2DplusT.nrrd");
  //MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(), "Testing whether BaseData is empty or not");
  //mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*> (image.GetPointer());
  //MITK_TEST_CONDITION_REQUIRED(image2.IsNotNull(), "Testing if BaseData is an image");

  // Delete this here because it will call the PrototypeServiceFactory::Unget() method
  // of the dummy writers.
  delete writerRegistry;

  //// always end with this!
  MITK_TEST_END()
}
