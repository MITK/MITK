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

#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"
#include "mitkImageStatisticsContainerManager.h"
#include "mitkProperties.h"

#include "QmitkImageStatisticsCalculationRunnable.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class TestQmitkImageStatisticsDataGenerator : public QmitkImageStatisticsDataGenerator
{
public:
  TestQmitkImageStatisticsDataGenerator(mitk::DataStorage::Pointer storage, QObject* parent = nullptr) : QmitkImageStatisticsDataGenerator(storage, parent)
  {
    connect(this, &QmitkDataGeneratorBase::NewDataAvailable, this, &TestQmitkImageStatisticsDataGenerator::NewDataAvailableEmited);
    connect(this, &QmitkDataGeneratorBase::DataGenerationStarted, this, &TestQmitkImageStatisticsDataGenerator::DataGenerationStartedEmited);
    connect(this, &QmitkDataGeneratorBase::GenerationFinished, this, &TestQmitkImageStatisticsDataGenerator::GenerationFinishedEmited);
    connect(this, &QmitkDataGeneratorBase::JobError, this, &TestQmitkImageStatisticsDataGenerator::JobErrorEmited);
  };

  mutable std::vector<mitk::DataStorage::SetOfObjects::ConstPointer> m_NewDataAvailable;
  void NewDataAvailableEmited(mitk::DataStorage::SetOfObjects::ConstPointer data) const
  {
    m_NewDataAvailable.emplace_back(data);
  };

  mutable int m_DataGenerationStartedEmited = 0;
  void DataGenerationStartedEmited(const mitk::DataNode* /*imageNode*/, const mitk::DataNode* /*roiNode*/, const QmitkDataGenerationJobBase* /*job*/) const
  {
    m_DataGenerationStartedEmited++;
  }

  mutable int m_GenerationFinishedEmited = 0;
  void GenerationFinishedEmited() const
  {
    m_GenerationFinishedEmited++;
    QCoreApplication::instance()->quit();
  }

  mutable std::vector<QString> m_JobErrorEmited_error;
  void JobErrorEmited(QString error, const QmitkDataGenerationJobBase* /*failedJob*/) const
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
    m_DataStorage->Add(m_ImageNode1);

    m_ImageNode2 = mitk::DataNode::New();
    m_ImageNode2->SetName("Image_2");
    m_Image2 = mitk::IOUtil::Load<mitk::Image>(pic3DCroppedFile);
    CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Image2.IsNotNull());
    m_ImageNode2->SetData(m_Image2);
    m_DataStorage->Add(m_ImageNode2);

    m_MaskImageNode = mitk::DataNode::New();
    m_MaskImageNode->SetName("Mask");
    auto pic3DCroppedBinMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedBinMask.nrrd");
    m_Mask = mitk::IOUtil::Load<mitk::Image>(pic3DCroppedBinMaskFile);
    CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D binary mask", m_Mask.IsNotNull());
    m_MaskImageNode->SetData(m_Mask);
    m_DataStorage->Add(m_MaskImageNode);

    m_PFNode = mitk::DataNode::New();
    m_PFNode->SetName("PF");
    auto pic3DCroppedPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedPF.pf");
    m_PF = mitk::IOUtil::Load<mitk::PlanarFigure>(pic3DCroppedPlanarFigureFile);
    CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D planar figure", m_PF.IsNotNull());
    m_PFNode->SetData(m_PF);
    m_DataStorage->Add(m_PFNode);

    int argc = 0;
    char** argv = nullptr;
    m_TestApp = new QCoreApplication(argc, argv);
  }

  void tearDown() override
  {
    delete m_TestApp;
  }


  bool CheckResultNode(const std::vector<mitk::DataNode::Pointer> resultNodes, const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, unsigned int histBin = 100, bool noZero = false)
  {
    for (auto& resultNode : resultNodes)
    {
      bool result = false;

      if (resultNode && resultNode->GetData() && imageNode && imageNode->GetData())
      {
        auto imageRule = mitk::StatisticsToImageRelationRule::New();
        result = !imageRule->GetRelationUIDs(resultNode, imageNode).empty();

        if (roiNode)
        {
          auto maskRule = mitk::StatisticsToMaskRelationRule::New();
          result = result && !maskRule->GetRelationUIDs(resultNode, roiNode).empty();
        }

        auto prop = resultNode->GetData()->GetProperty(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str());
        auto binProp = dynamic_cast<const mitk::UIntProperty*>(prop.GetPointer());
        result = result && binProp->GetValue() == histBin;

        prop = resultNode->GetData()->GetProperty(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str());
        auto zeroProp = dynamic_cast<const mitk::BoolProperty*>(prop.GetPointer());
        result = result && zeroProp->GetValue() == noZero;
      }

      if (result)
      { //node was in the result set
        return true;
      }

    }

    return false;
  }

  void NullTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(nullptr);

    generator.Generate();
    CPPUNIT_ASSERT_EQUAL(0, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(0, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(generator.m_NewDataAvailable.empty());

    generator.SetDataStorage(m_DataStorage);
    generator.Generate();
    CPPUNIT_ASSERT_EQUAL(0, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(0, generator.m_GenerationFinishedEmited);
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
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes { m_ImageNode1 };

    generator.SetImageNodes(imageNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(1, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(1 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable.front()->front() }, m_ImageNode1, nullptr));

    CPPUNIT_ASSERT_MESSAGE("Error: Rerun has generated new data.", generator.Generate());
  }

  void MultiImageTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes { m_ImageNode1, m_ImageNode2 };

    generator.SetImageNodes(imageNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(2, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(2u == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front() }, m_ImageNode1, nullptr));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front() }, m_ImageNode2, nullptr));

    CPPUNIT_ASSERT_MESSAGE("Error: Rerun has generated new data.", generator.Generate());
  }

  void ImageAndROITest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes { m_ImageNode1 };
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType roiNodes { m_MaskImageNode };

    generator.SetImageNodes(imageNodes);
    generator.SetROINodes(roiNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(1, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(1 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front() }, m_ImageNode1, m_MaskImageNode));

    CPPUNIT_ASSERT_MESSAGE("Error: Rerun has generated new data.", generator.Generate());

    roiNodes = { m_PFNode };

    generator.SetROINodes(roiNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(2, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(2, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(2 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[1]->front() }, m_ImageNode1, m_PFNode));

    CPPUNIT_ASSERT_MESSAGE("Error: Rerun has generated new data.", generator.Generate());
  }

  void ImageAndMultiROITest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes{ m_ImageNode1 };
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType roiNodes{ m_PFNode, m_MaskImageNode, nullptr };

    generator.SetImageNodes(imageNodes);
    generator.SetROINodes(roiNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(3, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(3 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(), generator.m_NewDataAvailable[2]->front()}, m_ImageNode1, m_PFNode));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(), generator.m_NewDataAvailable[2]->front() }, m_ImageNode1, m_MaskImageNode));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(), generator.m_NewDataAvailable[2]->front() }, m_ImageNode1, nullptr));

    CPPUNIT_ASSERT_MESSAGE("Error: Rerun has generated new data.", generator.Generate());
  }

  void MultiMultiTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes{ m_ImageNode1, m_ImageNode2 };
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType roiNodes{ m_PFNode, m_MaskImageNode, nullptr };

    generator.SetImageNodes(imageNodes);
    generator.SetROINodes(roiNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(6, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(6 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(),
                                     generator.m_NewDataAvailable[2]->front(), generator.m_NewDataAvailable[3]->front(),
                                     generator.m_NewDataAvailable[4]->front(), generator.m_NewDataAvailable[5]->front() }, m_ImageNode1, m_PFNode));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(),
                                     generator.m_NewDataAvailable[2]->front(), generator.m_NewDataAvailable[3]->front(),
                                     generator.m_NewDataAvailable[4]->front(), generator.m_NewDataAvailable[5]->front() }, m_ImageNode1, m_MaskImageNode));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(),
                                     generator.m_NewDataAvailable[2]->front(), generator.m_NewDataAvailable[3]->front(),
                                     generator.m_NewDataAvailable[4]->front(), generator.m_NewDataAvailable[5]->front() }, m_ImageNode1, nullptr));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(),
                                     generator.m_NewDataAvailable[2]->front(), generator.m_NewDataAvailable[3]->front(),
                                     generator.m_NewDataAvailable[4]->front(), generator.m_NewDataAvailable[5]->front() }, m_ImageNode2, m_PFNode));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(),
                                     generator.m_NewDataAvailable[2]->front(), generator.m_NewDataAvailable[3]->front(),
                                     generator.m_NewDataAvailable[4]->front(), generator.m_NewDataAvailable[5]->front() }, m_ImageNode2, m_MaskImageNode));
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front(), generator.m_NewDataAvailable[1]->front(),
                                     generator.m_NewDataAvailable[2]->front(), generator.m_NewDataAvailable[3]->front(),
                                     generator.m_NewDataAvailable[4]->front(), generator.m_NewDataAvailable[5]->front() }, m_ImageNode2, nullptr));

    CPPUNIT_ASSERT_MESSAGE("Error: Rerun has generated new data.", generator.Generate());
  }

  void InputChangedTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes{ m_ImageNode2 };

    generator.SetImageNodes(imageNodes);
    m_TestApp->processEvents();

    CPPUNIT_ASSERT_EQUAL(0, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(0, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(0 == generator.m_NewDataAvailable.size());

    imageNodes = { m_ImageNode1 };

    generator.SetAutoUpdate(true);
    generator.SetImageNodes(imageNodes);
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 1, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", 1 == generator.m_NewDataAvailable.size());

    generator.SetImageNodes(imageNodes);
    m_TestApp->processEvents();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 1, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 1 == generator.m_NewDataAvailable.size());

    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType roiNodes{ m_MaskImageNode };

    generator.SetAutoUpdate(true);
    generator.SetROINodes(roiNodes);
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 2, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 2, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", 2 == generator.m_NewDataAvailable.size());
  }

  void SettingsChangedTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes{ m_ImageNode1 };

    generator.SetImageNodes(imageNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(1, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(1 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[0]->front() }, m_ImageNode1, nullptr));

    generator.SetHistogramNBins(50);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(2, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(2, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(2 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[1]->front() }, m_ImageNode1, nullptr, 50));

    generator.SetIgnoreZeroValueVoxel(true);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(3, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(3, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(3 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[2]->front() }, m_ImageNode1, nullptr, 50, true));

    //now check auto update feature
    generator.SetAutoUpdate(true);
    generator.SetHistogramNBins(5);
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 4, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 4, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", 4 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[3]->front() }, m_ImageNode1, nullptr, 5, true));

    generator.SetHistogramNBins(5);
    m_TestApp->processEvents();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 4, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 4, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 4 == generator.m_NewDataAvailable.size());

    generator.SetIgnoreZeroValueVoxel(false);
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 5, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 5, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", 5 == generator.m_NewDataAvailable.size());
    CPPUNIT_ASSERT(CheckResultNode({ generator.m_NewDataAvailable[4]->front() }, m_ImageNode1, nullptr, 5, false));

    generator.SetIgnoreZeroValueVoxel(false);
    m_TestApp->processEvents();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 5, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 5, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but input does not realy changed.", 5 == generator.m_NewDataAvailable.size());
  }

  void DataStorageModificationTest()
  {
    TestQmitkImageStatisticsDataGenerator generator(m_DataStorage);
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType imageNodes{ m_ImageNode1 };
    QmitkImageAndRoiDataGeneratorBase::ConstNodeVectorType roiNodes{ m_PFNode };

    generator.SetImageNodes(imageNodes);
    generator.SetROINodes(roiNodes);
    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(1, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(1 == generator.m_NewDataAvailable.size());

    m_PF->Modified();
    m_PFNode->Modified();
    m_TestApp->processEvents();

    CPPUNIT_ASSERT_EQUAL(1, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(1, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(1 == generator.m_NewDataAvailable.size());

    generator.Generate();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL(2, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL(2, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT(generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT(2 == generator.m_NewDataAvailable.size());

    //now check auto update feature
    generator.SetAutoUpdate(true);
    m_PF->Modified();
    m_PFNode->Modified();
    m_TestApp->exec();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 3, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update seemed not to work.", 3, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update seemed not to work.", 3 == generator.m_NewDataAvailable.size());


    m_DataStorage->Add(mitk::DataNode::New());
    m_TestApp->processEvents();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", 3, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", 3, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", 3 == generator.m_NewDataAvailable.size());

    m_Image2->Modified();
    m_ImageNode2->Modified();
    m_TestApp->processEvents();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", 3, generator.m_DataGenerationStartedEmited);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", 3, generator.m_GenerationFinishedEmited);
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", generator.m_JobErrorEmited_error.empty());
    CPPUNIT_ASSERT_MESSAGE("Error: Auto update was triggerd, but only irrelevant node was added.", 3 == generator.m_NewDataAvailable.size());
  }

};

MITK_TEST_SUITE_REGISTRATION(QmitkImageStatisticsDataGenerator)
