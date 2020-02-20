/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataStorageListModel.h>
#include <mitkImage.h>
#include <mitkNodePredicateDataType.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkSurface.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

//! Tests for QmitkDataStorageListModel
//!
//! \todo could need quite some more tests, feel free to add test cases
class QmitkDataStorageListModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkDataStorageListModelTestSuite);
  MITK_TEST(InstantiationTest);
  MITK_TEST(FlexibleInputTest);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    mitk::Image::Pointer image = mitk::Image::New();
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData(image);

    mitk::Surface::Pointer surface = mitk::Surface::New();
    mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
    surfaceNode->SetData(surface);

    m_DataStorage->Add(imageNode);
    m_DataStorage->Add(surfaceNode);
  }

  void tearDown() override {}
  //! Test the c'tor variants
  void InstantiationTest()
  {
    QmitkDataStorageListModel emptyModel;
    CPPUNIT_ASSERT_EQUAL(0, emptyModel.rowCount());

    QmitkDataStorageListModel unfilteredModel(m_DataStorage);
    CPPUNIT_ASSERT_EQUAL(2, unfilteredModel.rowCount());

    mitk::TNodePredicateDataType<mitk::Image>::Pointer imagePredicate =
      mitk::TNodePredicateDataType<mitk::Image>::New();
    QmitkDataStorageListModel imageFilteredModel(m_DataStorage, imagePredicate.GetPointer());
    CPPUNIT_ASSERT_EQUAL(1, imageFilteredModel.rowCount());
  }

  //! Test that all combinations of
  //! - data storage
  //! - predicate
  //! are evaluated, cause reasonable row numbers, and do not crash
  void FlexibleInputTest()
  {
    QmitkDataStorageListModel model;
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), model.GetDataNodes().size());

    mitk::TNodePredicateDataType<mitk::Image>::Pointer imagePredicate =
      mitk::TNodePredicateDataType<mitk::Image>::New();

    model.SetPredicate(imagePredicate.GetPointer());
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), model.GetDataNodes().size());

    model.SetDataStorage(m_DataStorage);
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), model.GetDataNodes().size());

    model.SetPredicate(nullptr);
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), model.GetDataNodes().size());

    model.SetDataStorage(nullptr);
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), model.GetDataNodes().size());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkDataStorageListModel)
