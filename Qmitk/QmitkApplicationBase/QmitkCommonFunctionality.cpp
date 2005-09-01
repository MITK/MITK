#include <qregexp.h>
#include <string>
#include "mitkPointSetWriter.h"
#include "mitkConfig.h"
#ifdef MBI_INTERNAL
#include "mitkVesselGraphData.h"
#include "mitkVesselGraphFileWriter.h"
#endif
#define QMITKCOMMONFUNCTIONALITYIMPLEMENTATION
#include "QmitkCommonFunctionality.h"
#include <mitkImageAccessByItk.h>
#include <mitkPicFileReader.h>

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
#define EXTERNAL_FILE_EXTENSIONS "All known formats(*.dcm *.DCM *.gdcm *.pic *.pic.gz *.png *.jpg *.tiff *.pvtk *.stl *.vtk *.vti *.hdr);;DICOM files(*.dcm *.DCM *.gdcm);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;Sets of 2D slices (*.pic *.pic.gz *.png *.dcm *.gdcm);;stl files (*.stl)"
#define INTERNAL_FILE_EXTENSIONS "all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.vti *.ves *.uvg *.dvg *.par *.dcm *.gdcm *.mhd *.hdr hpsonos.db HPSONOS.DB *.png *.tiff *.jpg);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg *.dvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm *.gdcm)"
#else
#define EXTERNAL_FILE_EXTENSIONS "All known formats(*.dcm *.DCM *.gdcm *.pic *.pic.gz *.png *.jpg *.tiff *.pvtk *.stl *.vtk *.hdr);;DICOM files(*.dcm *.DCM *.gdcm);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;Sets of 2D slices (*.pic *.pic.gz *.png *.dcm *.gdcm);;stl files (*.stl)"
#define INTERNAL_FILE_EXTENSIONS "all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.dvg *.par *.gdcm *.dcm *.mhd *.hdr hpsonos.db HPSONOS.DB *.png *.tiff *.jpg);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg *.dvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm *.gdcm)"
#endif
#define SAVE_FILE_EXTENSIONS "all (*.pic *.mhd *.png *.tiff *.jpg)"

const char* CommonFunctionality::GetInternalFileExtensions() { return INTERNAL_FILE_EXTENSIONS; };
const char* CommonFunctionality::GetExternalFileExtensions() { return EXTERNAL_FILE_EXTENSIONS; };
const char* CommonFunctionality::GetSaveFileExtensions() { return SAVE_FILE_EXTENSIONS; };


#ifdef MBI_INTERNAL
/**
 * Saves the given directed vessel graph to a file. If no name is provided, the
 * user is prompted for a file name.
 */
void CommonFunctionality::SaveDirectedVesselGraph( mitk::DirectedVesselGraphData* graph, const char* aFileName )
{
  if(graph == NULL)
  {
    std::cout << "Warning in file " << __FILE__<< " line " << __LINE__ <<": vessel graph is NULL!" << std::endl;
    return;
  }
  QString fileName;
  if (aFileName == NULL)
  {
    fileName = QFileDialog::getSaveFileName(QString("VesselGraph.dvg"),"MITK VesselGraph (*.dvg)");
  }
  else
    fileName = aFileName;

  if (fileName.isEmpty() == false )
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
void CommonFunctionality::SaveUndirectedVesselGraph( mitk::UndirectedVesselGraphData* graph, const char* aFileName)
{
  if(graph == NULL)
  {
    std::cout << "Warning in file " << __FILE__<< " line " << __LINE__ <<": vessel graph is NULL!" << std::endl;
    return;
  }
  QString fileName;
  if (aFileName == NULL)
  {
    fileName = QFileDialog::getSaveFileName(QString("VesselGraph.uvg"),"MITK VesselGraph (*.uvg)");
  }
  else
    fileName = aFileName;

  if (fileName.isEmpty() == false )
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
void CommonFunctionality::SaveBaseData( mitk::BaseData* data, const char * aFileName )
{
  if (data != NULL)
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
    if(image.IsNotNull())
    {
      CommonFunctionality::SaveImage(image);
    }

    mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data);
    if(pointset.IsNotNull())
    {
      std::string fileName;
      if(aFileName == NULL)
        fileName = "PointSet";
      else
        fileName = aFileName;
      fileName = itksys::SystemTools::GetFilenameWithoutExtension(fileName);
      fileName += ".mps";
      QString qfileName = QFileDialog::getSaveFileName(QString(fileName.c_str()),"MITK Point-Sets (*.mps)");
      if (fileName.empty() == false )
      {
        mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
        writer->SetInput( pointset );
        writer->SetFileName( qfileName.ascii() );
        writer->Update();
      }
    }

    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data);
    if(surface.IsNotNull())
    {
      CommonFunctionality::SaveSurface(surface, aFileName);
    }

#ifdef MBI_INTERNAL
    mitk::UndirectedVesselGraphData::Pointer uvg = dynamic_cast<mitk::UndirectedVesselGraphData*>(data);
    if (uvg.IsNotNull())
    {
      CommonFunctionality::SaveUndirectedVesselGraph(uvg.GetPointer(), aFileName);
    }

    mitk::DirectedVesselGraphData::Pointer dvg = dynamic_cast<mitk::DirectedVesselGraphData*>(data);
    if (dvg.IsNotNull())
    {
      CommonFunctionality::SaveDirectedVesselGraph(dvg.GetPointer(), aFileName);
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


mitk::DataTreeNode::Pointer CommonFunctionality::FileOpenImageSequence(const char* aFileName)
{
  if(aFileName==NULL) return NULL;

  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  QString fileName = aFileName;
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
    factory->SetFilePattern( fileName.ascii() );
    factory->SetFilePrefix( fileName.ascii() );
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
static mitk::DataTreeIteratorBase* GetIteratorToFirstImage(mitk::DataTreeIteratorBase* dataTreeIterator)
{
  mitk::DataTreeIteratorClone it = dataTreeIterator;
  while ( !it->IsAtEnd() )
  {
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node->GetData() != NULL )
    {
      // access the original image
      mitk::Image::Pointer img = dynamic_cast<mitk::Image*>( node->GetData() );

      // enquiry whether img is NULL
      if ( img.IsNotNull() )
      {
        return it->Clone();
      }
    }
    ++it;
  }
  std::cout << "No node containing an mitk::Image found, returning NULL..." << std::endl;
  return NULL;
}

mitk::DataTreeIteratorBase* CommonFunctionality::GetIteratorToFirstImageInDataTree(mitk::DataTree::Pointer dataTree)
{
  mitk::DataTreePreOrderIterator dataTreeIterator( dataTree );

  if ( dataTree.IsNull() )
  {
    std::cout << "iterator to data tree is NULL. I cannot work without datatree !!"  << std::endl;
    return NULL;
  }

  return GetIteratorToFirstImage(&dataTreeIterator);
}

mitk::Image* CommonFunctionality::GetFirstImageInDataTree(mitk::DataTree::Pointer dataTree)
{
  mitk::DataTreeIteratorClone it = GetIteratorToFirstImageInDataTree(dataTree);
  if(it.IsNull())
    return NULL;
  return static_cast<mitk::Image*>(it->Get()->GetData());
}
mitk::DataTreeNode* CommonFunctionality::GetFirstNodeByProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
{
  mitk::DataTreeIteratorClone pos = dynamic_cast<mitk::DataTree*>( it->GetTree() )->GetNext( propertyKey.c_str(), property, it.GetPointer() );
  if ( ! pos->IsAtEnd() )
  {
    return pos->Get();
  }
  else
  {
    return NULL;
  }

}
mitk::BaseData* CommonFunctionality::GetFirstDataByProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
{
  mitk::DataTreeNode* node = GetFirstNodeByProperty( it, propertyKey, property );
  if ( node == NULL )
  {
    return NULL;
  }
  else
  {
    return node->GetData();
  }
}

mitk::DataTreeNode* CommonFunctionality::GetNodeForData( mitk::DataTreeIteratorClone it, mitk::BaseData* data )
{
  if ( it.GetPointer() == NULL )
  {
    return NULL;
  }

  mitk::DataTreeIteratorClone iteratorClone = it;
  while ( !iteratorClone->IsAtEnd() )
  {
    mitk::DataTreeNode::Pointer node = iteratorClone->Get();
    if ( node.IsNotNull() )
    {
      if ( node->GetData() == data )
        return node.GetPointer();
    }
    ++iteratorClone;
  }
  return NULL;
}

CommonFunctionality::DataTreeIteratorVector CommonFunctionality::FilterNodes(mitk::DataTreeIteratorClone it, bool (* FilterFunction)(mitk::DataTreeNode*))
{

  DataTreeIteratorVector result;

  if ( it.GetPointer() != NULL )
  {

    mitk::DataTreeIteratorClone iteratorClone = it;
    while ( !iteratorClone->IsAtEnd() )
    {
      mitk::DataTreeNode::Pointer node = iteratorClone->Get();
      if ( FilterFunction( node ) )
      {
        result.push_back(iteratorClone);
      }
      ++iteratorClone;
    }
  }
  return result;

}

#include "mitkSurfaceVtkWriter.h"
#include <vtkSTLWriter.h>
#include <vtkPolyDataWriter.h>

std::string CommonFunctionality::SaveSurface(mitk::Surface* surface, const char* aFileName)
{
  std::string fileName;
  if(aFileName == NULL)
    fileName = "Surface";
  else
    fileName = aFileName;

  std::string selectedItemsName = itksys::SystemTools::GetFilenameWithoutExtension(fileName);
  selectedItemsName += ".stl";
  QString qfileName = QFileDialog::getSaveFileName(QString(selectedItemsName.c_str()),"Surface Data(*.stl *.vtk)");
  if (qfileName != NULL )
  {
    if(qfileName.endsWith(".stl")==true)
    {
      mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
      writer->SetInput( surface );
      writer->SetFileName(qfileName.latin1());
      writer->GetVtkWriter()->SetFileTypeToBinary();
      writer->Write();
    }
    else
    {
      if (qfileName.endsWith(".vtk")==false)
        qfileName += ".vtk";
      mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
      writer->SetInput( surface );
      writer->SetFileName(qfileName.latin1());
      writer->Write();
    }
  }
  fileName = qfileName.ascii();
  return fileName;
}

#include "mitkImageWriter.h"
#include <itksys/SystemTools.hxx>

std::string CommonFunctionality::SaveImage(mitk::Image* image, const char* aFileName)
{
  std::string fileName;
  if(aFileName == NULL)
  {
    QString qfileName = QFileDialog::getSaveFileName(QString("NewImage.pic"),GetSaveFileExtensions());
    if (qfileName == NULL )
      return "";
    fileName = qfileName.ascii();
  }
  else
    fileName = aFileName;

  try
  {
    std::string dir = itksys::SystemTools::GetFilenamePath( fileName );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( fileName );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( fileName );

    dir += "/";
    dir += baseFilename;

    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
    imageWriter->SetInput(image);
    imageWriter->SetFileName(dir.c_str());
    imageWriter->SetExtension(extension.c_str());
    imageWriter->Write();
  }
  catch ( itk::ExceptionObject &err)
  {
    itkGenericOutputMacro( << "Exception during write: " << err );
    return "";
  }
  catch ( ... )
  {
    itkGenericOutputMacro( << "Unknown type of exception during write" );
  }
  return fileName;
}
