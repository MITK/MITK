/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsDataGenerator.h"
#include <QApplication>
#include <mitkStandaloneDataStorage.h>
#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include "mitkIOUtil.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class TestQmitkImageStatisticsDataGenerator : public QmitkImageStatisticsDataGenerator
{
public:
  TestQmitkImageStatisticsDataGenerator(mitk::DataStorage::Pointer storage, QObject* parent = nullptr) : QmitkImageStatisticsDataGenerator(storage, parent)
  {
    connect(this, &QmitkDataGeneratorBase::NewDataAvailable, this, &TestQmitkImageStatisticsDataGenerator::NewDataAvailableEmited);
    connect(this, &QmitkDataGeneratorBase::GenerationStarted, this, &TestQmitkImageStatisticsDataGenerator::GenerationStartedEmited);
    connect(this, &QmitkDataGeneratorBase::GenerationFinished, this, &TestQmitkImageStatisticsDataGenerator::GenerationFinishedEmited);
    connect(this, &QmitkDataGeneratorBase::JobError, this, &TestQmitkImageStatisticsDataGenerator::JobErrorEmited);
  };

  mutable std::vector<mitk::DataStorage::SetOfObjects::ConstPointer> m_NewDataAvailable;
  void NewDataAvailableEmited(mitk::DataStorage::SetOfObjects::ConstPointer data) const
  {
    m_NewDataAvailable.emplace_back(data);
  };

  mutable int m_GenerationStartedEmited = 0;
  void GenerationStartedEmited() const
  {
    m_GenerationStartedEmited++;
  }

  mutable int m_GenerationFinishedEmited = 0;
  void GenerationFinishedEmited() const
  {
    m_GenerationFinishedEmited++;
    QCoreApplication::instance()->quit();
  }

  mutable std::vector<QString> m_JobErrorEmited_error;
  void JobErrorEmited(QString error, const QmitkDataGenerationJobBase* failedJob) const
  {
    m_JobErrorEmited_error.emplace_back(error);
  }

};

class QmitkImageStatisticsDataGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkImageStatisticsDataGeneratorTestSuite);
  MITK_TEST(GetterSetterTest);
  MITK_TEST(NullTest);
  MITK_TEST(OneImageTest);
  MITK_TEST(MultiImageTest);
  MITK_TEST(ImageAndROITest);
  MITK_TEST(ImageAndMultiROITest);
  MITK_TEST(MultiMultiTest);
  MITK_TEST(InputChangedTest);
  MITK_TEST(SettingsChangedTest);
  MITK_TEST(DataStorageModificationTest);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_ImageNode1;
  mitk::DataNode::Pointer m_ImageNode2;
  mitk::DataNode::Pointer m_MaskImageNode;
  mitk::DataNode::Pointer m_PFNode;
  mitk::Image::Pointer m_Image1;
  mitk::Image::Pointer m_Image2;
  mitk::Image::Pointer m_Mask;
  mitk::PlanarFigure::Pointer m_PF;

  QCoreApplication* m_TestApp;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();

    m_ImageNode1 = mitk::DataNode::New();
    m_ImageNode1->SetName("Image_1");
    auto pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
    m_Image1 = mitk::IOUtil::Load<mitk::Image>(pic3DCroppedFile);
    CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Image1.IsNotNull());
    m_ImageNode1->SetData(m_Image1);

    m_ImageNode2 = mitk::DataNode::New();
    m_ImageNode2->SetName("Image_2");
    m_Image2 = mitk::IOUtil::Load<mitk::Image>(pic3DCroppedFile);
    CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Image2.IsNotNull());
    m_ImageNode2->SetData(m_Image2);

    m_MaskImageNode = mitk::DataNode::New();
    m_MaskImageNode->SetName("Mask");
    auto pic3DCroppedBinMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedBinMask.nrrd");
    m_Mask = mitk::IOUtil::Load<mitk::Image>(pic3DCroppedBinMaskFile);
    CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D binary mask", m_Mask.IsNotNull());
    m_MaskImageNode->SetData(m_Mask);

    m_PFNode = mitk::DataNode::New();
    m_PFNode->SetName("PF");
    auto pic3DCroppedPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedPF.pf");
    m_PF = mitk::IOUtil::Load<mitk::PlanarFigure>(pic3DCroppedPlanarFigureFile);
    CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D planar figure", m_PF.IsNotNull());
    m_PFNode->SetData(m_PF);

    int argc = 0;
    char** argv = nullptr;
    m_TestApp = new QCoreApplication(argc, argv);
  }

  void tearDown() override
  {
    delete m_TestApp;
  }


  void NullTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(nullptr);

    generator.Generate();
    CPPUNIT_ASSERT_EQUAL(0, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_EQUAL(0, generator.m_GenerationStartedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(generator.m_NewDataAvailable.empty());

    generator.SetDataStorage(m_DataStorage);
    generator.Generate();
    CPPUNIT_ASSERT_EQUAL(0, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_EQUAL(0, generator.m_GenerationStartedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(generator.m_NewDataAvailable.empty());
  }

  void GetterSetterTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(nullptr);
    CPPUNIT_ASSERT(nullptr == generator.GetDataStorage());
    generator.SetDataStorage(m_DataStorage);
    CPPUNIT_ASSERT(m_DataStorage == generator.GetDataStorage());

    TestQmitkImageStatisticsDataGenerator generator2(m_DataStorage);
    CPPUNIT_ASSERT(m_DataStorage == generator.GetDataStorage());

    CPPUNIT_ASSERT_EQUAL(100u, generator.GetHistogramNBins());
    CPPUNIT_ASSERT_EQUAL(false, generator.GetIgnoreZeroValueVoxel());
    CPPUNIT_ASSERT_EQUAL(false, generator.GetAutoUpdate());

    generator.SetHistogramNBins(3);
    CPPUNIT_ASSERT_EQUAL(3u, generator.GetHistogramNBins());
    CPPUNIT_ASSERT_EQUAL(false, generator.GetIgnoreZeroValueVoxel());
    CPPUNIT_ASSERT_EQUAL(false, generator.GetAutoUpdate());

    generator.SetIgnoreZeroValueVoxel(true);
    CPPUNIT_ASSERT_EQUAL(3u, generator.GetHistogramNBins());
    CPPUNIT_ASSERT_EQUAL(true, generator.GetIgnoreZeroValueVoxel());
    CPPUNIT_ASSERT_EQUAL(false, generator.GetAutoUpdate());

    generator.SetAutoUpdate(true);
    CPPUNIT_ASSERT_EQUAL(3u, generator.GetHistogramNBins());
    CPPUNIT_ASSERT_EQUAL(true, generator.GetIgnoreZeroValueVoxel());
    CPPUNIT_ASSERT_EQUAL(true, generator.GetAutoUpdate());
  }

  void OneImageTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);

    generator.SetImageNodes({ m_ImageNode1.GetPointer() });
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationStartedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(generator.m_NewDataAvailable.empty());
  }

  void MultiImageTest()
  {
  }

  void ImageAndROITest()
  {
  }

  void ImageAndMultiROITest()
  {
  }

  void MultiMultiTest()
  {
  }

  void InputChangedTest()
  {
  }

  void SettingsChangedTest()
  {
  }

  void DataStorageModificationTest()
  {
  }

};

MITK_TEST_SUITE_REGISTRATION(QmitkImageStatisticsDataGenerator)
