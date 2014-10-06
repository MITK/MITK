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
#include "mitkTestingConfig.h"

#include "mitkSceneIO.h"

#include "mitkStandaloneDataStorage.h"
#include "mitkStandardFileLocations.h"
#include "mitkCoreObjectFactory.h"
#include "mitkBaseData.h"
#include "mitkImage.h"
#include "mitkSurface.h"
#include "mitkPointSet.h"
#include "mitkIOUtil.h"
#include "Poco/File.h"
#include "Poco/TemporaryFile.h"

#ifndef WIN32
  #include <ulimit.h>
  #include <errno.h>
#endif

class SceneIOTestClass
{
  public:

static mitk::Image::Pointer LoadImage(const std::string& filename)
{
  mitk::Image::Pointer image = mitk::IOUtil::LoadImage( filename );
  if(image.IsNull())
  {
    MITK_TEST_FAILED_MSG(<< "Test image '" << filename << "' was not loaded as an mitk::Image");
  }
  return image;
}

static mitk::Surface::Pointer LoadSurface(const std::string& filename)
{
  mitk::Surface::Pointer surface = mitk::IOUtil::LoadSurface( filename );
  if(surface.IsNull())
  {
    MITK_TEST_FAILED_MSG(<< "Test surface '" << filename << "' was not loaded as an mitk::Surface");
  }
  return surface;
}

static mitk::PointSet::Pointer CreatePointSet()
{

  mitk::PointSet::Pointer ps = mitk::PointSet::New();
  mitk::PointSet::PointType p;
  mitk::FillVector3D(p, 1.0, -2.0, 33.0);
  ps->SetPoint(0, p);
  mitk::FillVector3D(p, 100.0, -200.0, 3300.0);
  ps->SetPoint(1, p);
  mitk::FillVector3D(p, 2.0, -3.0, 22.0);
  ps->SetPoint(2, p, mitk::PTCORNER); // add point spec
  //mitk::FillVector3D(p, -2.0, -2.0, -2.22);
  //ps->SetPoint(0, p, 1); // ID 0 in timestep 1
  //mitk::FillVector3D(p, -1.0, -1.0, -11.22);
  //ps->SetPoint(1, p, 1); // ID 1 in timestep 1
  //mitk::FillVector3D(p, 1000.0, 1000.0, 1122.22);
  //ps->SetPoint(11, p, mitk::PTCORNER, 2); // ID 11, point spec, timestep 2
  return ps;
}

static void FillStorage(mitk::DataStorage* storage, std::string imageName, std::string surfaceName)
{
  mitk::Image::Pointer image = LoadImage(imageName);
  MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(),"Loading test image"+imageName);

  image->SetProperty("image type", mitk::StringProperty::New("test image") );
  image->SetProperty("greetings", mitk::StringProperty::New("to mom") );
  image->SetProperty("test_float_property", mitk::FloatProperty::New(-2.57f));

  mitk::DataNode::Pointer imagenode = mitk::DataNode::New();
  imagenode->SetData( image );
  imagenode->SetName( "Pic3D" );
  storage->Add( imagenode );

  mitk::DataNode::Pointer imagechildnode = mitk::DataNode::New();
  imagechildnode->SetData( image );
  imagechildnode->SetName( "Pic3D again" );
  storage->Add( imagechildnode, imagenode );


  mitk::Surface::Pointer surface = LoadSurface(surfaceName );
  MITK_TEST_CONDITION_REQUIRED(surface.IsNotNull(),"Loading test surface binary.stl");

  surface->SetProperty("surface type", mitk::StringProperty::New("test surface") );
  surface->SetProperty("greetings", mitk::StringProperty::New("to dad") );

  mitk::DataNode::Pointer surfacenode = mitk::DataNode::New();
  surfacenode->SetData( surface );
  surfacenode->SetName( "binary" );
  storage->Add( surfacenode );

  mitk::PointSet::Pointer ps = CreatePointSet();
  mitk::DataNode::Pointer psenode = mitk::DataNode::New();
  psenode->SetData( ps );
  psenode->SetName( "points" );
  storage->Add( psenode );

}

static void VerifyStorage(mitk::DataStorage* storage)
{
  //TODO the Surface and PointSet are uncommented until the material property is saved properly
  mitk::DataNode::Pointer imagenode = storage->GetNamedNode("Pic3D");
  MITK_TEST_CONDITION_REQUIRED(imagenode.IsNotNull(),"Get previously stored image node");

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(imagenode->GetData());
  MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(),"Loading test image from Datastorage");

  //Image
  std::string testString("");
  float testFloatValue = 0.0f;
  mitk::PropertyList::Pointer imagePropList = image->GetPropertyList();

  imagePropList->GetStringProperty("image type", testString);
  MITK_TEST_CONDITION(testString == "test image" ,"Get StringProperty from previously stored image");

  imagePropList->GetStringProperty("greetings", testString);
  MITK_TEST_CONDITION(testString == "to mom" ,"Get another StringProperty from previously stored image");

  imagePropList->GetFloatProperty("test_float_property", testFloatValue);
  MITK_TEST_CONDITION(testFloatValue == -2.57f, "Get FloatProperty from previously stored image.")

  //Get Image child node
  mitk::DataNode::Pointer imagechildnode = storage->GetNamedNode("Pic3D again");
  mitk::DataStorage::SetOfObjects::ConstPointer objects = storage->GetSources(imagechildnode);

  MITK_TEST_CONDITION_REQUIRED(objects->Size() == 1,"Check size of image child nodes source list");
  MITK_TEST_CONDITION_REQUIRED(objects->ElementAt(0) == imagenode,"Check for right parent node");

  mitk::Image::Pointer imagechild = dynamic_cast<mitk::Image*>(imagechildnode->GetData());
  MITK_TEST_CONDITION_REQUIRED(imagechild.IsNotNull(),"Loading child test image from Datastorage");

  //Surface
  mitk::DataNode::Pointer surfacenode = storage->GetNamedNode("binary");
  MITK_TEST_CONDITION_REQUIRED(surfacenode.IsNotNull(),"Get previously stored surface node");

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(surfacenode->GetData());
  MITK_TEST_CONDITION_REQUIRED(surface.IsNotNull(),"Loading test surface from Datastorage");

  // Get the property list and test the properties
  mitk::PropertyList::Pointer surfacePropList = surface->GetPropertyList();
  surfacePropList->GetStringProperty("surface type", testString);
  MITK_TEST_CONDITION((testString.compare("test surface") == 0) ,"Get StringProperty from previously stored surface node");

  surfacePropList->GetStringProperty("greetings", testString);
  MITK_TEST_CONDITION((testString.compare("to dad") == 0) ,"Get another StringProperty from previously stored surface node");

  //PointSet
  mitk::DataNode::Pointer pointsnode = storage->GetNamedNode("points");
  MITK_TEST_CONDITION_REQUIRED(pointsnode.IsNotNull(),"Get previously stored PointSet node");

  mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(pointsnode->GetData());
  MITK_TEST_CONDITION_REQUIRED(pointset.IsNotNull(),"Loading test PointSet from Datastorage");

  mitk::PointSet::PointType p = pointset->GetPoint(0);
  MITK_TEST_CONDITION_REQUIRED(p[0] == 1.0 && p[1] == -2.0 && p[2] == 33.0, "Test Pointset entry 0 after loading");

  p = pointset->GetPoint(1);
  MITK_TEST_CONDITION_REQUIRED(p[0] == 100.0 && p[1] == -200.0 && p[2] == 3300.0, "Test Pointset entry 1 after loading");

  p = pointset->GetPoint(2);
  MITK_TEST_CONDITION_REQUIRED(p[0] == 2.0 && p[1] == -3.0 && p[2] == 22.0, "Test Pointset entry 2 after loading");

}
}; // end test helper class

int mitkSceneIOTest(int, char* argv[])
{
  MITK_TEST_BEGIN("SceneIO")
  std::string sceneFileName;

  for (unsigned int i = 0; i < 1; ++i) // TODO change to " < 2" to check cases where file system would be full
  {
    if (i == 1)
    {
      // call ulimit and restrict maximum file size to something small
      #ifndef WIN32
        errno = 0;
        long int value = ulimit(UL_SETFSIZE, 1);
        MITK_TEST_CONDITION_REQUIRED( value != -1, "ulimit() returned with errno = " << errno );
      #else
        continue;
      #endif
    }

    // create a data storage and fill it with some test data
    mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();
    MITK_TEST_CONDITION_REQUIRED(sceneIO.IsNotNull(),"SceneIO instantiation")

    mitk::DataStorage::Pointer storage = mitk::StandaloneDataStorage::New().GetPointer();
    MITK_TEST_CONDITION_REQUIRED(storage.IsNotNull(),"StandaloneDataStorage instantiation");

    std::cout << "ImageName: " << argv[1] << std::endl;
    std::cout << "SurfaceName: " << argv[2] << std::endl;

    SceneIOTestClass::FillStorage(storage, argv[1], argv[2]);

    // attempt to save it
    Poco::Path newname( Poco::TemporaryFile::tempName() );
    sceneFileName = std::string( MITK_TEST_OUTPUT_DIR ) + Poco::Path::separator() + newname.getFileName() + ".zip";
    MITK_TEST_CONDITION_REQUIRED( sceneIO->SaveScene( storage->GetAll(), storage, sceneFileName), "Saving scene file '" << sceneFileName << "'");

    // test if no errors were reported
    mitk::SceneIO::FailedBaseDataListType::ConstPointer failedNodes = sceneIO->GetFailedNodes();
    if (failedNodes.IsNotNull() && !failedNodes->empty())
    {
      MITK_TEST_OUTPUT( << "The following nodes could not be serialized:");
      for ( mitk::SceneIO::FailedBaseDataListType::const_iterator iter = failedNodes->begin();
            iter != failedNodes->end();
            ++iter )
      {
        MITK_TEST_OUTPUT_NO_ENDL( << " - ");
        if ( mitk::BaseData* data =(*iter)->GetData() )
        {
          MITK_TEST_OUTPUT_NO_ENDL( << data->GetNameOfClass());
        }
        else
        {
          MITK_TEST_OUTPUT_NO_ENDL( << "(NULL)");
        }

        MITK_TEST_OUTPUT( << " contained in node '" << (*iter)->GetName() << "'");
        // \TODO: should we fail the test case if failed properties exist?
      }
    }

    mitk::PropertyList::ConstPointer failedProperties = sceneIO->GetFailedProperties();
    if (failedProperties.IsNotNull() && !failedProperties->IsEmpty())
    {
      MITK_TEST_OUTPUT( << "The following properties could not be serialized:");
      const mitk::PropertyList::PropertyMap* propmap = failedProperties->GetMap();
      for ( mitk::PropertyList::PropertyMap::const_iterator iter = propmap->begin();
            iter != propmap->end();
            ++iter )
      {
        MITK_TEST_OUTPUT( << " - " << iter->second->GetNameOfClass() << " associated to key '" << iter->first << "'");
        // \TODO: should we fail the test case if failed properties exist?
      }
    }
    MITK_TEST_CONDITION_REQUIRED(failedProperties.IsNotNull() && failedProperties->IsEmpty(), "Checking if all properties have been saved.")
    MITK_TEST_CONDITION_REQUIRED(failedNodes.IsNotNull() && failedNodes->empty(), "Checking if all nodes have been saved.")

    //Now do the loading part
    sceneIO = mitk::SceneIO::New();

    //Load scene into the datastorage and clean the DS first
    MITK_TEST_OUTPUT(<< "Loading scene again");
    storage = sceneIO->LoadScene(sceneFileName,storage,true);

    // test if no errors were reported
    failedNodes = sceneIO->GetFailedNodes();
    if (failedNodes.IsNotNull() && !failedNodes->empty())
    {
      MITK_TEST_OUTPUT( << "The following nodes could not be serialized:");
      for ( mitk::SceneIO::FailedBaseDataListType::const_iterator iter = failedNodes->begin();
            iter != failedNodes->end();
            ++iter )
      {
        MITK_TEST_OUTPUT_NO_ENDL( << " - ");
        if ( mitk::BaseData* data =(*iter)->GetData() )
        {
          MITK_TEST_OUTPUT_NO_ENDL( << data->GetNameOfClass());
        }
        else
        {
          MITK_TEST_OUTPUT_NO_ENDL( << "(NULL)");
        }

        MITK_TEST_OUTPUT( << " contained in node '" << (*iter)->GetName() << "'");
        // \TODO: should we fail the test case if failed properties exist?
      }
    }

    failedProperties = sceneIO->GetFailedProperties();
    if (failedProperties.IsNotNull() && !failedProperties->IsEmpty())
    {
      MITK_TEST_OUTPUT( << "The following properties could not be serialized:");
      const mitk::PropertyList::PropertyMap* propmap = failedProperties->GetMap();
      for ( mitk::PropertyList::PropertyMap::const_iterator iter = propmap->begin();
            iter != propmap->end();
            ++iter )
      {
        MITK_TEST_OUTPUT( << " - " << iter->second->GetNameOfClass() << " associated to key '" << iter->first << "'");
        // \TODO: should we fail the test case if failed properties exist?
      }
    }

    // check if data storage content has been restored correctly
    SceneIOTestClass::VerifyStorage(storage);

  }
  // if no sub-test failed remove the scene file, otherwise it is kept for debugging purposes
  if ( mitk::TestManager::GetInstance()->NumberOfFailedTests() == 0 )
  {
    Poco::File pocoSceneFile( sceneFileName );
    MITK_TEST_CONDITION_REQUIRED( pocoSceneFile.exists(), "Checking if scene file still exists before cleaning up." )
    pocoSceneFile.remove();
  }
  MITK_TEST_END();
}

