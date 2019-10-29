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
// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

//MITK includes
#include <mitkIOUtil.h>
#include <mitkImageStatisticsContainerManager.h>
#include <mitkDataStorage.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkImageStatisticsContainer.h>
#include <mitkPlanarCircle.h>
#include <mitkImageStatisticsContainerNodeHelper.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>

class mitkImageStatisticsContainerManagerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageStatisticsContainerManagerTestSuite);
  MITK_TEST(GetImageStatisticsNoRules);
  MITK_TEST(GetImageStatisticsWithImageConnected);
  MITK_TEST(GetImageStatisticsWithImageNotConnected);
  MITK_TEST(GetImageStatisticsWithImageAndMaskConnected);
  MITK_TEST(GetImageStatisticsWithImageAndMaskNotConnected);
  MITK_TEST(GetImageStatisticsInvalid);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::ImageStatisticsContainer::Pointer m_statisticsContainer, m_statisticsContainer2, m_statisticsContainer3;
  mitk::Image::Pointer m_image, m_image2;
  mitk::Image::Pointer m_mask, m_mask2;
  mitk::PlanarFigure::Pointer m_planarFigure, m_planarFigure2;

public:
  void setUp() override
  {
    m_statisticsContainer = mitk::ImageStatisticsContainer::New();
    m_statisticsContainer2 = mitk::ImageStatisticsContainer::New();
    m_statisticsContainer3 = mitk::ImageStatisticsContainer::New();
    m_image = mitk::Image::New();
    m_image2 = mitk::Image::New();
    m_mask = mitk::Image::New();
    m_mask2 = mitk::Image::New();
    m_planarFigure = mitk::PlanarCircle::New().GetPointer();
    m_planarFigure2 = mitk::PlanarCircle::New().GetPointer();
  }

  void tearDown() override
  {
  }

  void CreateNodeRelationImage(mitk::BaseData::Pointer statistics, mitk::BaseData::ConstPointer image)
  {
    auto rule = mitk::StatisticsToImageRelationRule::New();
    rule->Connect(statistics, image);
  }

  void CreateNodeRelationMask(mitk::BaseData::Pointer statistics, mitk::BaseData::ConstPointer image)
  {
    auto rule = mitk::StatisticsToMaskRelationRule::New();
    rule->Connect(statistics, image);
  }

  void GetImageStatisticsNoRules() {
    auto statisticsNode = mitk::CreateImageStatisticsNode(m_statisticsContainer, "testStatistics");
    auto standaloneDataStorage = mitk::StandaloneDataStorage::New();
    standaloneDataStorage->Add(statisticsNode);

    //no rules + 1 image --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer emptyStatistic;
    CPPUNIT_ASSERT_NO_THROW(emptyStatistic = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(emptyStatistic.IsNull(), true);

    //no rules + 1 image + 1 mask --> test return nullptr
    CPPUNIT_ASSERT_NO_THROW(emptyStatistic = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_mask.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(emptyStatistic.IsNull(), true);

    //no rules + 1 image + 1 planarFigure --> test return nullptr
    CPPUNIT_ASSERT_NO_THROW(emptyStatistic = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_planarFigure.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(emptyStatistic.IsNull(), true);
  }

  void GetImageStatisticsWithImageConnected()
  {
    //create rules connection
    auto statisticsNode = mitk::CreateImageStatisticsNode(m_statisticsContainer, "testStatistics");
    CreateNodeRelationImage(m_statisticsContainer.GetPointer(), m_image.GetPointer());
    auto standaloneDataStorage = mitk::StandaloneDataStorage::New();
    standaloneDataStorage->Add(statisticsNode);

    //rule: (image-->statistics), 1 connected image --> test return image statistics
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImage;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImage = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImage->GetUID(), m_statisticsContainer->GetUID());

    //new rule: (image2-->statistics2 AND mask --> statistics2)
    CreateNodeRelationImage(m_statisticsContainer2.GetPointer(), m_image2.GetPointer());
    CreateNodeRelationMask(m_statisticsContainer2.GetPointer(), m_mask.GetPointer());

    auto statisticsNode2 = mitk::CreateImageStatisticsNode(m_statisticsContainer2, "testStatistics2");
    standaloneDataStorage->Add(statisticsNode2);

    //--> test return (still) image statistics (!= statistics2)
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAgain;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAgain = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAgain->GetUID(), m_statisticsContainer->GetUID());
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAgain->GetUID() != m_statisticsContainer2->GetUID(), true);

    //--> test return image statistics 2
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAgain = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image2.GetPointer(), m_mask.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAgain->GetUID(), m_statisticsContainer2->GetUID());
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAgain->GetUID() != m_statisticsContainer->GetUID(), true);

    //add another newer statistic: should return this newer one
    auto statisticsContainerNew = mitk::ImageStatisticsContainer::New();
    CreateNodeRelationImage(statisticsContainerNew.GetPointer(), m_image.GetPointer());

    auto statisticsNodeNew = mitk::CreateImageStatisticsNode(statisticsContainerNew, "testStatisticsNew");
    standaloneDataStorage->Add(statisticsNodeNew);
    statisticsContainerNew->Modified();

    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageNew;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageNew = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageNew->GetUID(), statisticsContainerNew->GetUID());
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageNew->GetUID() != m_statisticsContainer->GetUID(), true);
  }

  void GetImageStatisticsWithImageNotConnected() {
    //create rules connection
    auto statisticsNode = mitk::CreateImageStatisticsNode(m_statisticsContainer, "testStatistics");
    CreateNodeRelationImage(m_statisticsContainer.GetPointer(), m_image.GetPointer());
    auto standaloneDataStorage = mitk::StandaloneDataStorage::New();
    standaloneDataStorage->Add(statisticsNode);

    //rule: (image-->statistics), 1 unconnected image --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImage;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImage = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image2.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImage.IsNull(), true);

    //rule: (image-->statistics), 1 connected image + 1 unconnected mask --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAndMask;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndMask = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_mask.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndMask.IsNull(), true);

    //rule: (image-->statistics), 1 connected image + 1 unconnected planar figure --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAndPlanarFigure;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndPlanarFigure = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_planarFigure.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndPlanarFigure.IsNull(), true);
  }

  void GetImageStatisticsWithImageAndMaskConnected()
  {
    //create rules connection + add statistics to dataStorage
    auto statisticsNode = mitk::CreateImageStatisticsNode(m_statisticsContainer, "testStatistics");

    CreateNodeRelationImage(m_statisticsContainer.GetPointer(), m_image.GetPointer());
    CreateNodeRelationMask(m_statisticsContainer.GetPointer(), m_mask.GetPointer());
    auto standaloneDataStorage = mitk::StandaloneDataStorage::New();
    standaloneDataStorage->Add(statisticsNode);

    //rule: (image-->statistics, mask-->statistics), 1 connected image, 1 connected mask --> test return statistics
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAndMask;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndMask = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_mask.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndMask.IsNull(), false);
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndMask->GetUID(), m_statisticsContainer->GetUID());

    //new rule: (image-->statistics2) --> returns statistic because statistic2 has no mask connection
    CreateNodeRelationImage(m_statisticsContainer2.GetPointer(), m_image.GetPointer());
    
    auto statisticsNode2 = mitk::CreateImageStatisticsNode(m_statisticsContainer2, "testStatistics2");
    standaloneDataStorage->Add(statisticsNode2);

    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAndMaskAgain;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndMaskAgain = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_mask.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndMaskAgain->GetUID(), m_statisticsContainer->GetUID());
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndMaskAgain->GetUID() != m_statisticsContainer2->GetUID(), true);

    //add another newer statistic: should return this newer one
    auto statisticsContainerNew = mitk::ImageStatisticsContainer::New();

    mitk::PropertyRelations::RuleResultVectorType rules4;
    CreateNodeRelationImage(statisticsContainerNew.GetPointer(), m_image.GetPointer());
    CreateNodeRelationMask(statisticsContainerNew.GetPointer(), m_mask.GetPointer());
    auto statisticsNodeNew = mitk::CreateImageStatisticsNode(statisticsContainerNew, "testStatisticsNew");
    standaloneDataStorage->Add(statisticsNodeNew);

    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAndMaskNew;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndMaskNew = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_mask.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsContainerNew->GetUID(), statisticsWithImageAndMaskNew->GetUID());
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndMaskNew->GetUID() != m_statisticsContainer->GetUID(), true);
  }

  void GetImageStatisticsWithImageAndMaskNotConnected()
  {
    //create rules connection + add statistics to dataStorage
    auto statisticsNode = mitk::CreateImageStatisticsNode(m_statisticsContainer, "testStatistics");
    CreateNodeRelationImage(m_statisticsContainer.GetPointer(), m_image.GetPointer());
    CreateNodeRelationMask(m_statisticsContainer.GetPointer(), m_mask.GetPointer());
    auto standaloneDataStorage = mitk::StandaloneDataStorage::New();
    standaloneDataStorage->Add(statisticsNode);

    //rule: (image-->statistics, mask-->statistics), 1 connected image --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImage;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImage = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImage.IsNull(), true);

    //rule: (image-->statistics, mask-->statistics), 1 unconnected image, 1 unconnected mask --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageNotConnectedAndMaskNotConnected;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageNotConnectedAndMaskNotConnected = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image2.GetPointer(), m_mask2.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageNotConnectedAndMaskNotConnected.IsNull(), true);

    //rule: (image-->statistics, mask-->statistics), 1 unconnected image, 1 connected mask --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAndMaskNotConnected;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndMaskNotConnected = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image2.GetPointer(), m_mask.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndMaskNotConnected.IsNull(), true);

    //rule: (image-->statistics, mask-->statistics), 1 connected image, 1 unconnected planarFigure --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageAndPlanarFigureNotConnected;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndPlanarFigureNotConnected = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image.GetPointer(), m_planarFigure.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndPlanarFigureNotConnected.IsNull(), true);

    //rule: (image-->statistics, mask-->statistics), 1 unconnected image, 1 unconnected planarFigure --> test return nullptr
    mitk::ImageStatisticsContainer::ConstPointer statisticsWithImageNotConnectedAndPlanarFigureNotConnected;
    CPPUNIT_ASSERT_NO_THROW(statisticsWithImageAndPlanarFigureNotConnected = mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), m_image2.GetPointer(), m_planarFigure.GetPointer()));
    CPPUNIT_ASSERT_EQUAL(statisticsWithImageAndPlanarFigureNotConnected.IsNull(), true);
  }

  void GetImageStatisticsInvalid()
  {
    CreateNodeRelationImage(m_statisticsContainer.GetPointer(), m_image.GetPointer());

    CPPUNIT_ASSERT_THROW(mitk::ImageStatisticsContainerManager::GetImageStatistics(nullptr, m_image.GetPointer()), mitk::Exception);

    auto standaloneDataStorage = mitk::StandaloneDataStorage::New();

    CPPUNIT_ASSERT_THROW(mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), nullptr), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), nullptr, m_mask.GetPointer()), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::ImageStatisticsContainerManager::GetImageStatistics(standaloneDataStorage.GetPointer(), nullptr, m_planarFigure.GetPointer()), mitk::Exception);
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkImageStatisticsContainerManager)