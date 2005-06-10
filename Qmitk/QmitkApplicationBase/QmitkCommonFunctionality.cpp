#include "mitkConfig.h"
#ifdef MBI_INTERNAL
#include "mitkVesselGraphData.h"
#include "mitkVesselGraphFileWriter.h"
#endif
#define QMITKCOMMONFUNCTIONALITYIMPLEMENTATION
#include "QmitkCommonFunctionality.h"

#ifdef MBI_INTERNAL
/**
 * Saves the given directed vessel graph to a file. If no name is provided, the
 * user is prompted for a file name.
 */
void CommonFunctionality::SaveDirectedVesselGraph( mitk::DirectedVesselGraphData* graph, std::string name )
{
  if(graph == NULL)
  {
    std::cout << "Warning in file " << __FILE__<< " line " << __LINE__ <<": vessel graph is NULL!" << std::endl;
    return;
  }
  QString fileName = name.c_str();
  if (fileName == "")
  {
    fileName = QFileDialog::getSaveFileName(QString("VesselGraph.dvg"),"MITK VesselGraph (*.dvg)");
  }
  if (fileName != NULL )
  {
    mitk::VesselGraphFileWriter<Directed>::Pointer writer = mitk::VesselGraphFileWriter<Directed>::New();
    writer->SetInput( graph );
    writer->SetFileName( fileName.ascii() );
    writer->Update();
  }
}

/**
 * Saves the given undirected vessel graph to a file. If no name is provided, the
 * user is prompted for a file name.
 */
void CommonFunctionality::SaveUndirectedVesselGraph( mitk::UndirectedVesselGraphData* graph, std::string name)
{
  if(graph == NULL)
  {
    std::cout << "Warning in file " << __FILE__<< " line " << __LINE__ <<": vessel graph is NULL!" << std::endl;
    return;
  }
  QString fileName = name.c_str();
  if (fileName == "")
  {
    fileName = QFileDialog::getSaveFileName(QString("VesselGraph.uvg"),"MITK VesselGraph (*.uvg)");
  }
  if (fileName != NULL )
  {
    mitk::VesselGraphFileWriter<Undirected>::Pointer writer = mitk::VesselGraphFileWriter<Undirected>::New();
    writer->SetInput( graph );
    writer->SetFileName( fileName.ascii() );
    writer->Update();
  }
}
#endif

/**
 * Saves the given mitk::BaseData to a file. The user is prompted to
 * enter a file name. Currently only mitk::Image, mitk::Surface, mitk::PointSet and
 * mitk::VesselGraphData are supported. This function is deprecated
 * until the save-problem is solved by means of a Save-Factory or any
 * other "nice" mechanism
 */
void CommonFunctionality::SaveBaseData( mitk::BaseData* data, std::string name )
{
  if (data != NULL)
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
    if(image.IsNotNull())
    {
      typedef itk::Image<int,3> ImageType;
      CommonFunctionality::SaveImage< ImageType >(image);
    }

    mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data);
    if(pointset.IsNotNull())
    {
      name = itksys::SystemTools::GetFilenameWithoutExtension(name);
      name += ".mps";
      QString fileName = QFileDialog::getSaveFileName(QString(name.c_str()),"MITK Point-Sets (*.mps)");
      if (fileName != NULL )
      {
        mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
        writer->SetInput( pointset );
        writer->SetFileName( fileName.ascii() );
        writer->Update();
      }
    }

    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data);
    if(surface.IsNotNull())
    {
      CommonFunctionality::SaveSurface(surface, "SurfaceModel.stl");
    }

#ifdef MBI_INTERNAL
    mitk::UndirectedVesselGraphData::Pointer uvg = dynamic_cast<mitk::UndirectedVesselGraphData*>(data);
    if (uvg.IsNotNull())
    {
      CommonFunctionality::SaveUndirectedVesselGraph(uvg.GetPointer());
    }

    mitk::DirectedVesselGraphData::Pointer dvg = dynamic_cast<mitk::DirectedVesselGraphData*>(data);
    if (dvg.IsNotNull())
    {
      CommonFunctionality::SaveDirectedVesselGraph(dvg.GetPointer());
    }
#endif

  }
}
mitk::DataTreeNode::Pointer CommonFunctionality::AddVtkMeshToDataTree(vtkPolyData* polys, mitk::DataTreeIteratorBase* iterator, std::string str)
{
  mitk::DataTreeIteratorClone it=iterator;

  mitk::DataTreeNode::Pointer node = NULL;
  mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", new mitk::StringProperty( str.c_str() ));

  if (subTree->IsAtEnd() || subTree->Get().IsNull() )
  {
    node=mitk::DataTreeNode::New();
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(str.c_str());
    node->SetProperty("name",nameProp);
    it->Add(node);
  }
  else
  {
    node = subTree->Get();
  }

  mitk::Surface::Pointer surface = mitk::Surface::New();
  surface->SetVtkPolyData(polys);
  node->SetData( surface );
  node->SetProperty("layer", new mitk::IntProperty(1));
  node->SetVisibility(true,NULL);

  float meshColor[3] = {1.0,0.5,0.5};
  node->SetColor(meshColor,  NULL );
  node->SetVisibility(true, NULL );

  return node;
}

mitk::DataTreeNode::Pointer CommonFunctionality::AddPicImageToDataTree(ipPicDescriptor * pic, mitk::DataTreeIteratorBase* iterator, std::string str)
{
  mitk::DataTreeIteratorClone it=iterator;

  mitk::Image::Pointer image = mitk::Image::New();
  image->Initialize(pic);
  image->SetPicVolume(pic);

  mitk::DataTreeNode::Pointer node = NULL;
  mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", new mitk::StringProperty( str.c_str() ));

  if (subTree->IsAtEnd() || subTree->Get().IsNull() )
  {
    node=mitk::DataTreeNode::New();
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(str.c_str());
    node->SetProperty("name",nameProp);
    node->SetData(image);
    it->Add(node);
  }
  else
  {
    node = subTree->Get();
    node->SetData(image);
  }

  mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
  mitk::LevelWindow levelWindow;
  levelWindow.SetAuto( image );
  levWinProp->SetLevelWindow(levelWindow);
  node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
  return node;
}

void CommonFunctionality::SetWidgetPlanesEnabled(mitk::DataTree* dataTree, bool enable)
{
  // widget plane are made visible again
  mitk::DataTreeIteratorClone it = dataTree->GetNext("name", new mitk::StringProperty("widget1Plane"));
  if (!it->IsAtEnd())
  {
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node.IsNotNull() )
    {
      node->SetVisibility(enable, NULL);
    }
  }
  it = dataTree->GetNext("name", new mitk::StringProperty("widget2Plane"));
  if (!it->IsAtEnd())
  {
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node.IsNotNull() )
    {
      node->SetVisibility(enable, NULL);
    }
  }
  it = dataTree->GetNext("name", new mitk::StringProperty("widget3Plane"));
  if (!it->IsAtEnd())
  {
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node.IsNotNull() )
    {
      node->SetVisibility(enable, NULL);
    }
  }
}
mitk::DataTreeNode::Pointer CommonFunctionality::FileOpen( const char * fileName )
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  try
  {
    factory->SetFileName( fileName );
    factory->Update();
    return factory->GetOutput( 0 );
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
    return NULL;
  }
}


mitk::DataTreeNode::Pointer CommonFunctionality::FileOpenImageSequence(QString fileName)
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  if (!fileName.contains("dcm") && !fileName.contains("DCM"))
  {
    int fnstart = fileName.findRev( QRegExp("[/\\\\]"), fileName.length() );
    if(fnstart<0) fnstart=0;
    int start = fileName.find( QRegExp("[0-9]"), fnstart );
    if(start<0)
    {
      return FileOpen(fileName.ascii());;
    }

    char prefix[1024], pattern[1024];

    strncpy(prefix, fileName.ascii(), start);
    prefix[start]=0;

    int stop=fileName.find( QRegExp("[^0-9]"), start );
    sprintf(pattern, "%%s%%0%uu%s",stop-start,fileName.ascii()+stop);


    factory->SetFilePattern( pattern );
    factory->SetFilePrefix( prefix );
  }
  else
  {
    //    factory->SetFileName( fileName );
    factory->SetFilePattern( fileName );
    factory->SetFilePrefix( fileName );
  }
  factory->Update();
  return factory->GetOutput( 0 );

}

mitk::DataTreeNode::Pointer CommonFunctionality::FileOpenImageSequence()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,GetExternalFileExtensions());

  if ( !fileName.isNull() )
  {
    return FileOpenImageSequence(fileName);
  }
  else
  {
    return NULL;
  }
}

mitk::DataTreeNode::Pointer CommonFunctionality::FileOpen()
{
#ifdef MBI_INTERNAL
  QString fileName = QFileDialog::getOpenFileName(NULL,GetInternalFileExtensions() );
#else
  QString fileName = QFileDialog::getOpenFileName(NULL,GetExternalFileExtensions() );
#endif
  if ( !fileName.isNull() )
  {
    mitk::DataTreeNode::Pointer result = FileOpen(fileName.ascii());
    if ( result.IsNull() )
    {
      return FileOpenImageSequence(fileName);
    }
    else
    {
      return result;
    }
  }
  else
  {
    return NULL;
  }
}

mitk::DataTreeNode::Pointer CommonFunctionality::OpenVolumeOrSliceStack()
{
  mitk::DataTreeNode::Pointer newNode = NULL;

  QString fileName = QFileDialog::getOpenFileName(NULL,GetExternalFileExtensions() );
  if ( !fileName.isNull() )
  {
    newNode = CommonFunctionality::FileOpen(fileName);
    if (newNode.IsNotNull())
    {
      mitk::Image::Pointer imageData = dynamic_cast<mitk::Image*> (newNode->GetData()) ;
      if (imageData->GetDimension(2) == 1)
      {
        //        std::string dir = itksys::SystemTools::GetFilenamePath( std::string(fileName.ascii()) )
        newNode = CommonFunctionality::FileOpenImageSequence(fileName);
        imageData = dynamic_cast<mitk::Image*> (newNode->GetData());
      }
      return newNode;
    }
  }
  {
    return NULL;
  }
}
