/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkTestingMacros.h"

#include "mitkSceneIO.h"

#include "mitkStandaloneDataStorage.h"
#include "mitkStandardFileLocations.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkCoreObjectFactory.h"
#include "mitkBaseData.h"
#include "mitkImage.h"
#include "mitkSurface.h"

class SceneIOTestClass
{
  public:

/// returns full path to a test file
static std::string LocateFile(const std::string& filename)
{
  mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(locator.IsNotNull(),"Instantiating StandardFileLocations") 
  return locator->FindFile(filename.c_str(), "Core/Testing/Data");
}

static mitk::BaseData::Pointer LoadBaseData(const std::string& filename)
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
  try
  {
    factory->SetFileName( filename );
    factory->Update();

    if(factory->GetNumberOfOutputs()<1)
    {
      MITK_TEST_FAILED_MSG(<< "Could not find test data '" << filename << "'");
    }

    mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
    return node->GetData();
  }
  catch ( itk::ExceptionObject & e )
  {
    MITK_TEST_FAILED_MSG(<< "Failed loading test data '" << filename << "': " << e.what());
  }
}

static mitk::Image::Pointer LoadImage(const std::string& filename)
{
  mitk::BaseData::Pointer basedata = LoadBaseData( filename );
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(basedata.GetPointer());
  if(image.IsNull())
  {
    MITK_TEST_FAILED_MSG(<< "Test image '" << filename << "' was not loaded as an mitk::Image");
  }
  return image;
}

static mitk::Surface::Pointer LoadSurface(const std::string& filename)
{
  mitk::BaseData::Pointer basedata = LoadBaseData( filename );
  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(basedata.GetPointer());
  if(surface.IsNull())
  {
    MITK_TEST_FAILED_MSG(<< "Test surface '" << filename << "' was not loaded as an mitk::Surface");
  }
  return surface;
}

static void FillStorage(mitk::DataStorage* storage)
{
  mitk::Image::Pointer image = LoadImage( LocateFile("Pic3D.pic.gz") );
  MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(),"Loading test image Pic3D.pic.gz");

  image->SetProperty("image type", mitk::StringProperty::New("test image") );
  image->SetProperty("greetings", mitk::StringProperty::New("to mom") );

  mitk::DataTreeNode::Pointer imagenode = mitk::DataTreeNode::New();
  imagenode->SetData( image );
  imagenode->SetName( "Pic3D" );
  storage->Add( imagenode );
  
  mitk::Surface::Pointer surface = LoadSurface( LocateFile("binary.stl") );
  MITK_TEST_CONDITION_REQUIRED(surface.IsNotNull(),"Loading test surface binary.stl");
  
  surface->SetProperty("surface type", mitk::StringProperty::New("test surface") );
  surface->SetProperty("greetings", mitk::StringProperty::New("to dad") );
  
  mitk::DataTreeNode::Pointer surfacenode = mitk::DataTreeNode::New();
  surfacenode->SetData( surface );
  surfacenode->SetName( "binary" );
  storage->Add( surfacenode );
}

}; // end test helper class
  
int mitkSceneIOTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("SceneIO")

  itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());
  
  mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();
  MITK_TEST_CONDITION_REQUIRED(sceneIO.IsNotNull(),"SceneIO instantiation") 
  
  mitk::DataStorage::Pointer storage = mitk::StandaloneDataStorage::New().GetPointer();
  MITK_TEST_CONDITION_REQUIRED(storage.IsNotNull(),"StandaloneDataStorage instantiation");

  SceneIOTestClass::FillStorage(storage);

  MITK_TEST_CONDITION_REQUIRED( sceneIO->SaveScene( storage, "scene.zip" ),
                                "Saving scene file" );

  MITK_TEST_END()
}

