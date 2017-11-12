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
#include <mitkTestFixture.h>

#include "mitkLevelWindowManager.h"
#include "mitkStandaloneDataStorage.h"
#include <itkComposeImageFilter.h>
#include <itkEventObject.h>
#include <itkImageDuplicator.h>
#include <itkImageIterator.h>
#include <mitkImageCast.h>

class mitkLevelWindowManagerCppUnitTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLevelWindowManagerCppUnitTestSuite);
  MITK_TEST(TestMultiComponentRescaling);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_mitkMultiComponentImage;
  mitk::Image::Pointer m_mitkImageComponent1;
  mitk::Image::Pointer m_mitkImageComponent2;

public:
  void setUp() override
  {
    typedef itk::Image<double, 3> ImageType;
    typedef itk::ImageRegionIterator<ImageType> ImageIteratorType;
    typedef itk::ImageDuplicator<ImageType> DuplicatorType;
    typedef itk::ComposeImageFilter<ImageType> CompositeFilterType;

    // generate two images with one component
    ImageType::Pointer imageComponent1 = itk::Image<double, 3>::New();
    ImageType::IndexType start;
    start.Fill(0);
    ImageType::SizeType size;
    size.Fill(5);
    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);
    imageComponent1->SetRegions(region);
    imageComponent1->Allocate();

    DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage(imageComponent1);
    duplicator->Update();
    ImageType::Pointer imageComponent2 = duplicator->GetOutput();

    // give them differing data
    ImageIteratorType iterator1(imageComponent1, imageComponent1->GetLargestPossibleRegion());
    iterator1.GoToBegin();
    int i = 0;
    while (!iterator1.IsAtEnd())
    {
      iterator1.Set((double)i);
      ++iterator1;
      ++i;
    }

    ImageIteratorType iterator2(imageComponent2, imageComponent2->GetLargestPossibleRegion());
    iterator2.GoToBegin();
    i = 2000;
    while (!iterator2.IsAtEnd())
    {
      iterator2.Set((double)i);
      ++iterator2;
      ++i;
    }

    // copy into single VectorImage
    CompositeFilterType::Pointer compositeFilter = CompositeFilterType::New();
    compositeFilter->SetInput(0, imageComponent1);
    compositeFilter->SetInput(1, imageComponent2);
    compositeFilter->Update();
    itk::VectorImage<double, 3>::Pointer multiComponentImage = compositeFilter->GetOutput();

    // cast images to mitk
    mitk::CastToMitkImage(multiComponentImage, m_mitkMultiComponentImage);
    mitk::CastToMitkImage(imageComponent1, m_mitkImageComponent1);
    mitk::CastToMitkImage(imageComponent2, m_mitkImageComponent2);
  }

  static mitk::LevelWindow getLevelWindowForImage(mitk::Image *image, unsigned component)
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

    try
    {
      manager->SetDataStorage(ds);
    }
    catch (std::exception &e)
    {
      MITK_ERROR << "Exception: " << e.what();
    }

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(image);
    ds->Add(node);

    node->SetBoolProperty("selected", true);
    node->SetIntProperty("Image.Displayed Component", component);

    mitk::LevelWindow levelWindow;
    node->GetLevelWindow(levelWindow);
    return levelWindow; // node is an image node because of predicates
  }

  void TestMultiComponentRescaling()
  {
    // compute level windows for the three images
    mitk::LevelWindow imageComponent1LevelWindow = getLevelWindowForImage(m_mitkImageComponent1, 0);
    mitk::LevelWindow imageComponent2LevelWindow = getLevelWindowForImage(m_mitkImageComponent2, 0);
    // calculate level window for second component in multi-component image
    mitk::LevelWindow multiComponentImageLevelWindow = getLevelWindowForImage(m_mitkMultiComponentImage, 1);

    // compare level window boundaries. the multicomponent image level window
    // should match the second image, since the second component was selected
    CPPUNIT_ASSERT_EQUAL_MESSAGE("default lower bounds equal",
                                 imageComponent2LevelWindow.GetDefaultLowerBound(),
                                 multiComponentImageLevelWindow.GetDefaultLowerBound());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("default upper bounds equal",
                                 imageComponent2LevelWindow.GetDefaultUpperBound(),
                                 multiComponentImageLevelWindow.GetDefaultUpperBound());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "range equal", imageComponent2LevelWindow.GetRange(), multiComponentImageLevelWindow.GetRange());

    CPPUNIT_ASSERT(imageComponent1LevelWindow.GetDefaultLowerBound() !=
                   multiComponentImageLevelWindow.GetDefaultLowerBound());

    CPPUNIT_ASSERT(imageComponent1LevelWindow.GetDefaultUpperBound() !=
                   multiComponentImageLevelWindow.GetDefaultUpperBound());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLevelWindowManagerCppUnit)
