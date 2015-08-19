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

#include "mitkLevelWindowManager.h"
#include "mitkStandaloneDataStorage.h"
#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include "mitkRenderingModeProperty.h"
#include <itkImageIterator.h>
#include <mitkImageCast.h>
#include <itkImageDuplicator.h>
#include <itkComposeImageFilter.h>
#include <itkEventObject.h>

class mitkLevelWindowManagerTestClass
{
public:

  static void TestInstantiation()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    MITK_TEST_CONDITION_REQUIRED(manager.IsNotNull(),"Testing mitk::LevelWindowManager::New()");
  }

  static void TestSetGetDataStorage()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    MITK_TEST_OUTPUT(<< "Creating DataStorage: ");
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

    bool success = true;
    try
    {
      manager->SetDataStorage(ds);
    }
    catch(std::exception e)
    {
      success = false;
      MITK_ERROR << "Exception: " << e.what();
    }
    MITK_TEST_CONDITION_REQUIRED(success,"Testing mitk::LevelWindowManager SetDataStorage() ");
    MITK_TEST_CONDITION_REQUIRED(ds == manager->GetDataStorage(),"Testing mitk::LevelWindowManager GetDataStorage ");

  }

  static void TestMethodsWithInvalidParameters()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    bool success = false;
    mitk::LevelWindowProperty::Pointer levelWindowProperty = mitk::LevelWindowProperty::New();
    try
    {
      manager->SetLevelWindowProperty(levelWindowProperty);
    }
    catch(mitk::Exception e)
    {
      success = true;
    }
    MITK_TEST_CONDITION(success,"Testing mitk::LevelWindowManager SetLevelWindowProperty with invalid parameter");
  }

  static void TestOtherMethods()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    MITK_TEST_CONDITION(manager->isAutoTopMost(),"Testing mitk::LevelWindowManager isAutoTopMost");

    // It is not clear what the following code is supposed to test. The expression in
    // the catch(...) block does have no effect, so success is always true.
    // Related bugs are 13894 and 13889
    /*
  bool success = true;
  try
  {
    mitk::LevelWindow levelWindow = manager->GetLevelWindow();
    manager->SetLevelWindow(levelWindow);
  }
  catch (...)
  {
    success == false;
  }
  MITK_TEST_CONDITION(success,"Testing mitk::LevelWindowManager GetLevelWindow() and SetLevelWindow()");
     */

    manager->SetAutoTopMostImage(true);
    MITK_TEST_CONDITION(manager->isAutoTopMost(),"Testing mitk::LevelWindowManager isAutoTopMost()");
  }

  static void TestRemoveObserver(std::string testImageFile)
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    //add multiple objects to the data storage => multiple observers should be created
    mitk::Image::Pointer image1 = mitk::IOUtil::LoadImage(testImageFile);
    mitk::DataNode::Pointer node1 = mitk::DataNode::New();
    node1->SetData(image1);
    mitk::Image::Pointer image2 = mitk::IOUtil::LoadImage(testImageFile);
    mitk::DataNode::Pointer node2 = mitk::DataNode::New();
    node2->SetData(image2);
    ds->Add(node1);
    ds->Add(node2);

    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 2, "Test if nodes have been added");
    MITK_TEST_CONDITION_REQUIRED(static_cast<int>(manager->GetRelevantNodes()->size()) == manager->GetNumberOfObservers(), "Test if number of nodes is similar to number of observers");

    mitk::Image::Pointer image3 = mitk::IOUtil::LoadImage(testImageFile);
    mitk::DataNode::Pointer node3 = mitk::DataNode::New();
    node3->SetData(image3);
    ds->Add(node3);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 3, "Test if another node have been added");
    MITK_TEST_CONDITION_REQUIRED(static_cast<int>(manager->GetRelevantNodes()->size()) == manager->GetNumberOfObservers(), "Test if number of nodes is similar to number of observers");

    ds->Remove(node1);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 2, "Deleted node 1 (test GetRelevantNodes())");
    MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 2, "Deleted node 1 (test GetNumberOfObservers())");

    ds->Remove(node2);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 1, "Deleted node 2 (test GetRelevantNodes())");
    MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 1, "Deleted node 2 (test GetNumberOfObservers())");

    ds->Remove(node3);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 0, "Deleted node 3 (test GetRelevantNodes())");
    MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 0, "Deleted node 3 (test GetNumberOfObservers())");

  }

  static bool VerifyRenderingModes()
  {
    bool ok = false;

    ok = ( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR == 1 ) &&
        (mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR == 2 ) &&
        (mitk::RenderingModeProperty::LOOKUPTABLE_COLOR == 3 ) &&
        (mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR == 4 );

    return ok;
  }

  static void TestLevelWindowSliderVisibility(std::string testImageFile)
  {
    bool renderingModesValid = mitkLevelWindowManagerTestClass::VerifyRenderingModes();
    if ( !renderingModesValid )
    {
      MITK_ERROR << "Exception: Image Rendering.Mode property value types inconsistent.";
    }

    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    //add multiple objects to the data storage => multiple observers should be created
    mitk::Image::Pointer image1 = mitk::IOUtil::LoadImage(testImageFile);
    mitk::DataNode::Pointer node1 = mitk::DataNode::New();
    node1->SetData(image1);
    ds->Add(node1);

    //mitk::DataNode::Pointer node1 = mitk::IOUtil::LoadDataNode( testImageFile );
    mitk::DataNode::Pointer node2 = mitk::IOUtil::Load( testImageFile, *ds )->GetElement(0);
    mitk::DataNode::Pointer node3 = mitk::IOUtil::Load( testImageFile, *ds )->GetElement(0);
    std::vector< mitk::DataNode::Pointer > nodeVec;
    //nodeVec.resize( 3 );
    nodeVec.push_back( node1 );
    nodeVec.push_back( node2 );
    nodeVec.push_back( node3 );

    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator RandomGeneratorType;
    RandomGeneratorType::Pointer rnd = RandomGeneratorType::New();
    rnd->Initialize();

    for( unsigned int i=0; i<8; ++i )
    {
      unsigned int parity = i;

      for( unsigned int img = 0; img < 3; ++img )
      {
        if ( parity & 1 )
        {
          int mode = rnd->GetIntegerVariate() % 3;
          nodeVec[img]->SetProperty( "Image Rendering.Mode", mitk::RenderingModeProperty::New( mode ) );
        }
        else
        {
          int mode = rnd->GetIntegerVariate() % 2;
          nodeVec[img]->SetProperty( "Image Rendering.Mode", mitk::RenderingModeProperty::New( 3 + mode ) );
        }
        parity >>= 1;
      }

      MITK_TEST_CONDITION( renderingModesValid && ( (!manager->GetLevelWindowProperty() && !i) || (manager->GetLevelWindowProperty() && i) ), "Testing level window property member according to rendering mode");
    }
  }


  static mitk::LevelWindow getLevelWindowForImage(mitk::Image* image, unsigned component)
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

    try
    {
      manager->SetDataStorage(ds);
    }
    catch(std::exception e)
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

  static void TestMultiComponentRescaling()
  {

    // generate two images
    typedef itk::VectorImage<double, 3> VectorImageType;


    typedef itk::Image<double, 3> ImageType;
    ImageType::Pointer imageComponent1 = itk::Image<double,3>::New();

    ImageType::IndexType start;
    start.Fill(0);
    ImageType::SizeType size;
    size.Fill(5);
    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    imageComponent1->SetRegions(region);
    imageComponent1->Allocate();
    imageComponent1->FillBuffer(1000.0);


    typedef itk::ImageDuplicator< ImageType > DuplicatorType;
    DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage(imageComponent1);
    duplicator->Update();
    ImageType::Pointer imageComponent2 = duplicator->GetOutput();

    // give them differing data
    typedef itk::ImageRegionIterator< ImageType > ImageIteratorType;


    ImageIteratorType iterator1(imageComponent1,
        imageComponent1->GetLargestPossibleRegion());
    iterator1.GoToBegin();
    int i = 0;
    while(!iterator1.IsAtEnd())
    {
      iterator1.Set((double) i);
      ++iterator1;
      ++i;
    }

    ImageIteratorType iterator2(imageComponent2,
        imageComponent2->GetLargestPossibleRegion());
    iterator2.GoToBegin();
    i = 2000;
    while(!iterator2.IsAtEnd())
    {
      iterator2.Set((double) i);
      ++iterator2;
      ++i;
    }

    // copy into one vectorimage
    typedef itk::ComposeImageFilter<ImageType> CompositeFilterType;
    CompositeFilterType::Pointer compositeFilter = CompositeFilterType::New();
    compositeFilter->SetInput( 0, imageComponent1 );
    compositeFilter->SetInput( 1, imageComponent2 );

    compositeFilter->Update();
    itk::VectorImage< double, 3 >::Pointer multiComponentImage=
        compositeFilter->GetOutput();


    // cast images to mitk
    mitk::Image::Pointer mitkMultiComponentImage;
    mitk::CastToMitkImage(multiComponentImage, mitkMultiComponentImage);

    mitk::Image::Pointer mitkImageComponent1;
    mitk::CastToMitkImage(imageComponent1, mitkImageComponent1);

    mitk::Image::Pointer mitkImageComponent2;
    mitk::CastToMitkImage(imageComponent2, mitkImageComponent2);


    // compute level windows for the three images

    mitk::LevelWindow imageComponent1LevelWindow =
        getLevelWindowForImage(mitkImageComponent1, 0);

    mitk::LevelWindow imageComponent2LevelWindow =
        getLevelWindowForImage(mitkImageComponent2, 0);

    // calculate level window for second component in multi-component image
    mitk::LevelWindow multiComponentImageLevelWindow =
        getLevelWindowForImage(mitkMultiComponentImage, 1);


    // compare level window boundaries. the multicomponent image level window
    // should match the second image, since the second component was selected

    MITK_TEST_CONDITION(imageComponent2LevelWindow.GetDefaultLowerBound() ==
        multiComponentImageLevelWindow.GetDefaultLowerBound(), "default lower bounds equal");

    MITK_TEST_CONDITION(imageComponent2LevelWindow.GetDefaultUpperBound() ==
        multiComponentImageLevelWindow.GetDefaultUpperBound(), "default upper bounds equal");

    MITK_TEST_CONDITION(imageComponent2LevelWindow.GetRange() ==
        multiComponentImageLevelWindow.GetRange(), "range equal");

    MITK_TEST_CONDITION(imageComponent1LevelWindow.GetDefaultLowerBound() !=
        multiComponentImageLevelWindow.GetDefaultLowerBound(), "lower bound not equal to first level window");

    MITK_TEST_CONDITION(imageComponent1LevelWindow.GetDefaultUpperBound() !=
        multiComponentImageLevelWindow.GetDefaultUpperBound(), "upper bound not equal to first level window");

  }

};

int mitkLevelWindowManagerTest(int argc, char* args[])
{
  MITK_TEST_BEGIN("mitkLevelWindowManager");

  MITK_TEST_CONDITION_REQUIRED( argc >= 2, "Testing if test file is given.");
  std::string testImage = args[1];

  mitkLevelWindowManagerTestClass::TestInstantiation();
  mitkLevelWindowManagerTestClass::TestSetGetDataStorage();
  mitkLevelWindowManagerTestClass::TestMethodsWithInvalidParameters();
  mitkLevelWindowManagerTestClass::TestOtherMethods();
  mitkLevelWindowManagerTestClass::TestRemoveObserver(testImage);
  mitkLevelWindowManagerTestClass::TestLevelWindowSliderVisibility(testImage);
  mitkLevelWindowManagerTestClass::TestMultiComponentRescaling();

  MITK_TEST_END();
}
