/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "QmitkSelectableGLWidget.h"
#include "QLevelWindowWidget.h"
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkImageData.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPointsWriter.h>
#include <map>
#include <mitkSurfaceData.h>

#include <mitkProperties.h>
#include <mitkLookupTableProperty.h>
#include <mitkColorProperty.h>
#include <mitkBoolProperty.h>
#include <mitkFloatProperty.h>
#include <mitkLevelWindowProperty.h>

#include <mitkLevelWindow.h>
#include <mitkDataTree.h>
#include <qregexp.h>
#include <string>

#include <mitkStringProperty.h>
#include <qstring.h>
#include "mitkImageTimeSelector.h"
#include "mitkImageChannelSelector.h"
//#include <mitkImageToItk.h>
//#include <itkThresholdImageFilter.h>

#include <mitkStateMachineFactory.h>
#include <mitkUndoController.h>
#include <mitkStateMachine.h>
#include <mitkEventMapper.h>
#include <mitkGlobalInteraction.h>

#include <mitkUSLookupTableSource.h>

#ifdef MBI_INTERNAL
#include <mitkVesselTreeFileReader.h>
#include <mitkVesselGraphFileReader.h>
#include <mitkDICOMFileReader.h>
#include <mitkDSRFileReader.h>
#include <mitkCylindricToCartesianFilter.h>
#include <itksys/SystemTools.hxx>
#else
  #include "itkImage.h"
  #include "itkImageFileReader.h"
  #include "itkDICOMImageIO2.h"
  #include "itkImageSeriesReader.h"
  #include "itkDICOMSeriesFileNames.h"
  #include "QmitkCommonFunctionality.h"
#endif

#include <mitkParRecFileReader.h>
#include <mitkInteractionConst.h>
#include <QmitkStatusBar/QmitkStatusBar.h>

void QmitkMainTemplate::fileNew()
{

}

void QmitkMainTemplate::fileOpen()
{
#ifdef MBI_INTERNAL
  QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.par *.dcm hpsonos.db HPSONOS.DB);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm)");
#else
  QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.par);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;par/rec files (*.par);;DICOM files (*.dcm)");
#endif

  if ( !fileName.isNull() )
  {
    fileOpen(fileName.ascii());
  }
}

void QmitkMainTemplate::fileOpen( const char * fileName )
{
  mitk::DataTreeIterator* it=NULL;
  mitk::DataTreeNode::Pointer node=NULL;
  if(strstr(fileName, ".stl")!=0)
  {
    std::cout << "loading " << fileName << " as stl ... " << std::endl;

    vtkSTLReader *stlreader = vtkSTLReader::New();
    stlreader->SetFileName(fileName);
    stlreader->Update();

    if(stlreader->GetOutput()!=NULL)
    {
      mitk::SurfaceData::Pointer surface = mitk::SurfaceData::New();
      surface->SetVtkPolyData(stlreader->GetOutput());

      it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      node->SetData(surface);
      it->add(node);
    }
  }
  else if(strstr(fileName, ".vtk")!=0)
  {
    std::cout << "loading " << fileName << " as vtk ... " << std::endl;

    vtkPolyDataReader *reader = vtkPolyDataReader::New();
    reader->SetFileName(fileName);
    reader->Update();

    if(reader->GetOutput()!=NULL)
    {
      mitk::SurfaceData::Pointer surface = mitk::SurfaceData::New();
      surface->SetVtkPolyData(reader->GetOutput());

      it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      node->SetData(surface);
      it->add(node);
    }
  }
  else if((strstr(fileName, ".pic")!=0) || (strstr(fileName, ".seq")!=0))
  {
    ipPicDescriptor *header=ipPicGetHeader(const_cast<char *>(fileName), NULL);
    if(header!=NULL)
    {
      mitk::PicFileReader::Pointer reader;

      reader=mitk::PicFileReader::New();

      std::cout << "loading " << fileName << " as pic ... " << std::endl;

      reader->SetFileName(fileName);

      it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      
      node->SetData(reader->GetOutput());

      it->add(node);

      QString tmpName = fileName;
      tmpName = tmpName.right(tmpName.length() - tmpName.findRev("/") - 1);
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(tmpName.ascii());
      node->SetProperty("fileName",nameProp);

      // disable volume rendering by default
      node->SetProperty("volumerendering",new mitk::BoolProperty(false));

      //	    mitk::USLookupTableSource::Pointer LookupTableSource = mitk::USLookupTableSource::New();
      //  	  LookupTableSource->SetUseStrainRateLookupTable();
      //	    LookupTableSource->Update();
      //  	  mitk::LookupTableSource::OutputTypePointer LookupTable = LookupTableSource->GetOutput();
      //	    mitk::LookupTableProperty::Pointer LookupTableProp = new mitk::LookupTableProperty(*LookupTable);
      //			node->SetProperty("LookupTable", LookupTableProp );

      mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
      mitk::LevelWindow levelWindow;
      reader->UpdateLargestPossibleRegion();
      levelWindow.SetAuto( reader->GetOutput()->GetPic() );
      levWinProp->SetLevelWindow(levelWindow);
      node->GetPropertyList()->SetProperty("levelwindow",levWinProp);

      mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);

      //mitkMultiWidget->initWidgets(it);
    }
  }
  else if(strstr(fileName, ".par")!=0)
  {

    mitk::ParRecFileReader::Pointer reader;

    reader=mitk::ParRecFileReader::New();
    std::cout << "loading " << fileName << " as par/rec ... " << std::endl;
    reader->SetFileName(fileName);

    it=tree->inorderIterator();

    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);

    // disable volume rendering by default
    node->SetProperty("volumerendering",new mitk::BoolProperty(false));

    QString tmpName = fileName;
    tmpName = tmpName.right(tmpName.length() - tmpName.findRev("/") - 1);
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(tmpName.ascii());
    node->SetProperty("fileName",nameProp);

    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelWindow;
    reader->UpdateLargestPossibleRegion();
    levelWindow.SetAuto( reader->GetOutput()->GetPic() );
    levWinProp->SetLevelWindow(levelWindow);
    node->GetPropertyList()->SetProperty("levelwindow",levWinProp);

    mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);
    //mitkMultiWidget->initWidgets(it);
  }
  else if(strstr(fileName, ".pvtk")!=0)
  {
    vtkStructuredPointsReader *vtkreader = vtkStructuredPointsReader::New();
    vtkreader->SetFileName(fileName);
    vtkreader->Update();

    if(vtkreader->GetOutput()!=NULL)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->Initialize(vtkreader->GetOutput());
      image->SetVolume(vtkreader->GetOutput()->GetScalarPointer());

      vtkreader->Delete();

      it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      node->SetData(image);
      it->add(node);

      // disable volume rendering by default
      node->SetProperty("volumerendering",new mitk::BoolProperty(false));

      QString tmpName = fileName;
      tmpName = tmpName.right(tmpName.length() - tmpName.findRev("/") - 1);
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(tmpName.ascii());
      node->SetProperty("fileName",nameProp);

      mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
      mitk::LevelWindow levelWindow;
      levelWindow.SetAuto( image->GetPic() );
      levWinProp->SetLevelWindow(levelWindow);
      node->GetPropertyList()->SetProperty("levelwindow",levWinProp);

      mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);
      //mitkMultiWidget->initWidgets(it);
    }
  }
#ifdef MBI_INTERNAL
  else if( strstr(fileName, ".ves")!=0 )
  {
    mitk::VesselTreeFileReader::Pointer reader = mitk::VesselTreeFileReader::New();
    std::cout << "loading " << fileName << " as ves ... " << std::endl;
    reader->SetFileName(fileName);
    reader->Update();
    it=tree->inorderIterator();

    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);
  }
  else if( strstr(fileName, ".uvg")!=0 )
  {
    mitk::VesselGraphFileReader::Pointer reader = mitk::VesselGraphFileReader::New();
    std::cout << "loading " << fileName << " as uvg ... " << std::endl;
    reader->SetFileName(fileName);
    reader->Update();
    it=tree->inorderIterator();

    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);
  }
  else if(strstr(fileName, "DCM")!=0 || strstr(fileName, "dcm")!=0)
  {
    mitk::DICOMFileReader::Pointer reader;

    reader=mitk::DICOMFileReader::New();
    std::cout << "loading " << fileName << " as DICOM... " << std::endl;

    reader->SetFileName(fileName);

    it=tree->inorderIterator();

    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);

    // disable volume rendering by default
    node->SetProperty("volumerendering",new mitk::BoolProperty(false));

    mitk::LevelWindow levelWindow;
    reader->Update();
    levelWindow.SetAuto( reader->GetOutput()->GetPic() );
    node->SetLevelWindow(levelWindow, NULL);
    mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);
  }
  else if(itksys::SystemTools::Strucmp(itksys::SystemTools::GetFilenameName(fileName).c_str(), "HPSONOS.DB")==0)
  {

    //if (strstr(fileName, "HPSONOS.DB")!=0)
    //{
    //  std::cout << "renamed HPSONOS.DB to hpsonos.db" << std::endl;
    //  std::string newFileName(fileName);

    //  int length = newFileName.length();
    //  newFileName.replace(length-10,10,"hpsonos.db");
    //  std::cout << "new filename: " << newFileName << std::endl;
    //  fileName = newFileName.c_str();
    //}


    mitk::DSRFileReader::Pointer reader;

    reader=mitk::DSRFileReader::New();
    std::cout << "loading " << fileName << " as DSR ... " << std::endl;

    reader->SetFileName(fileName);

    mitk::ImageChannelSelector::Pointer channelSelector=mitk::ImageChannelSelector::New();
    mitk::ImageChannelSelector::Pointer DopplerChannelSelector=mitk::ImageChannelSelector::New();

    channelSelector->SetInput(reader->GetOutput());
    DopplerChannelSelector->SetInput(reader->GetOutput());

    reader->UpdateOutputInformation();
    bool haveDoppler = false;
    if (reader->GetOutput()->IsValidChannel(0))
    {
      std::cout << "    have channel data 0 (backscatter) ... " << std::endl;
    }

    if (reader->GetOutput()->IsValidChannel(1))
    {
      std::cout << "    have channel data 1 (doppler) ... " << std::endl;
      haveDoppler = true;
    }

    mitk::CylindricToCartesianFilter::Pointer cyl2cart = mitk::CylindricToCartesianFilter::New();;
    mitk::CylindricToCartesianFilter::Pointer cyl2cartDoppler = mitk::CylindricToCartesianFilter::New();

    cyl2cart->SetTargetXSize(128);
    cyl2cartDoppler->SetTargetXSize(128); 

    it=tree->inorderIterator();

    //
    // switch to Backscatter information
    //    
    channelSelector->SetChannelNr(0);
    //channelSelector->Update();
    //ipPicPut("timeselect.pic",channelSelector->GetOutput()->GetPic());    

    mitk::ImageSliceSelector::Pointer sliceSelector=mitk::ImageSliceSelector::New();
    //
    // insert original (in cylinric coordinates) Backscatter information
    //
    node = mitk::DataTreeNode::New();
    node->SetData(channelSelector->GetOutput());
    sliceSelector->SetInput(channelSelector->GetInput());
    mitk::StringProperty::Pointer ultrasoundProp = new mitk::StringProperty("OriginalBackscatter");    
    node->SetProperty("ultrasound",ultrasoundProp);
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty("OriginalBackscatter");
    node->SetProperty("fileName",nameProp);
    node->SetProperty("layer", new mitk::IntProperty(-11) );
    mitk::LevelWindow levelWindow;
    levelWindow.SetAuto( sliceSelector->GetOutput()->GetPic() );
    node->SetLevelWindow(levelWindow, NULL);
    node->SetVisibility(false,NULL);
    node->Update();
    it->add(node);    
    // disable volume rendering by default
    node->SetProperty("volumerendering",new mitk::BoolProperty(false));

    //
    // insert transformed (in cartesian coordinates) Backscatter information
    //
    cyl2cart->SetInput(reader->GetOutput());
    node=mitk::DataTreeNode::New();
    node->SetData(cyl2cart->GetOutput());
    ultrasoundProp = new mitk::StringProperty("TransformedBackscatter");
    node->SetProperty("ultrasound",ultrasoundProp);
    nameProp = new mitk::StringProperty("TransformedBackscatter");
    node->SetProperty("fileName",nameProp);
    node->SetProperty("layer", new mitk::IntProperty(-10) );
    //    mitk::LevelWindow levelWindow;

    sliceSelector->SetInput(cyl2cart->GetOutput());
    sliceSelector->Update();
    levelWindow.SetAuto( sliceSelector->GetOutput()->GetPic() );
    node->SetLevelWindow(levelWindow, NULL);
    node->Update();
    it->add(node);    
    // disable volume rendering by default
    node->SetProperty("volumerendering",new mitk::BoolProperty(false));

    if (haveDoppler)
    {

      //
      // switch to Doppler information
      //
      DopplerChannelSelector->SetChannelNr(1);
      //DopplerChannelSelector->Update();

      //
      // create a Doppler lookup table
      // TODO: HP map must be provided by DSRFilereader, since it
      // may be dependend on data ( baseline shift)
      //
      mitk::USLookupTableSource::Pointer LookupTableSource = mitk::USLookupTableSource::New();
      LookupTableSource->SetUseDSRDopplerLookupTable();
      LookupTableSource->Update();
      mitk::LookupTableSource::OutputTypePointer LookupTable = LookupTableSource->GetOutput();
      mitk::LookupTableProperty::Pointer LookupTableProp = new mitk::LookupTableProperty(*LookupTable);


      //
      // insert original (in cylindric coordinates) Doppler information
      //
      node=mitk::DataTreeNode::New();
      node->SetData(DopplerChannelSelector->GetOutput());
      ultrasoundProp = new mitk::StringProperty("OriginalDoppler");
      node->SetProperty("ultrasound",ultrasoundProp);
      nameProp = new mitk::StringProperty("OriginalDoppler");
      node->SetProperty("fileName",nameProp);
      node->SetProperty("layer", new mitk::IntProperty(-6) );


      mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
      mitk::LevelWindow levelWindow;
      levelWindow.SetLevelWindow(128, 255);
      levWinProp->SetLevelWindow(levelWindow);      
      // set the overwrite LevelWindow
      // if "levelwindow" is used if "levelWindow" is not available
      // else "levelWindow" is used
      // "levelWindow" is not affected by the slider
      node->GetPropertyList()->SetProperty("levelWindow",levWinProp);

      node->SetProperty("LookupTable", LookupTableProp );
      node->SetVisibility(false,NULL);
      node->Update();
      it->add(node);
      // disable volume rendering by default
      node->SetProperty("volumerendering",new mitk::BoolProperty(false));

      //
      // insert transformed (in cartesian coordinates) Doppler information
      //
      cyl2cartDoppler->SetInput(DopplerChannelSelector->GetOutput());
      node=mitk::DataTreeNode::New();
      node->SetData(cyl2cartDoppler->GetOutput());
      ultrasoundProp = new mitk::StringProperty("TransformedDoppler");
      node->SetProperty("ultrasound",ultrasoundProp);
      nameProp = new mitk::StringProperty("TransformedDoppler");
      node->SetProperty("fileName",nameProp);
      node->SetProperty("layer", new mitk::IntProperty(-5) );
      cyl2cartDoppler->Update();

      // set the overwrite LevelWindow
      // if "levelwindow" is used if "levelWindow" is not available
      // else "levelWindow" is used
      // "levelWindow" is not affected by the slider
      node->GetPropertyList()->SetProperty("levelWindow",levWinProp);

      node->SetProperty("LookupTable", LookupTableProp );
      node->Update();
      it->add(node);
      // disable volume rendering by default
      node->SetProperty("volumerendering",new mitk::BoolProperty(false));
    }

    mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);
  }
#else
  if(strstr(fileName, "DCM")!=0 || strstr(fileName, "dcm")!=0)
  {
    QString tmpName = fileName;
    QString dir = tmpName.left(tmpName.findRev("/",tmpName.length())+1);

    typedef itk::Image<unsigned short,3>            ImageType;
    typedef itk::ImageSeriesReader< ImageType >     ReaderType;

    itk::DICOMImageIO2::Pointer dicomIO = itk::DICOMImageIO2::New();

    // Get the DICOM filenames from the directory
    itk::DICOMSeriesFileNames::Pointer nameGenerator = itk::DICOMSeriesFileNames::New();
    nameGenerator->SetDirectory( dir.ascii() );

    typedef std::vector<std::string> seriesIdContainer;
    const seriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();

    seriesIdContainer::const_iterator seriesItr = seriesUID.begin();
    seriesIdContainer::const_iterator seriesEnd = seriesUID.end();

    std::cout << std::endl << "The directory: " << std::endl;
    std::cout << std::endl << dir << std::endl << std::endl;
    std::cout << "Contains the following DICOM Series: ";
    std::cout << std::endl << std::endl;

    while( seriesItr != seriesEnd )
    {
      std::cout << seriesItr->c_str() << std::endl;
      seriesItr++;
    }

    std::cout << std::endl << std::endl;
    std::cout << "Now reading series: " << std::endl << std::endl;

    typedef std::vector<std::string> fileNamesContainer;
    fileNamesContainer fileNames;

    std::cout << seriesUID.begin()->c_str() << std::endl;
    fileNames = nameGenerator->GetFileNames();

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileNames( fileNames );
    reader->SetImageIO( dicomIO );

    try
    {
      reader->Update();
    }
    catch (itk::ExceptionObject &ex)
    {
      std::cout << ex << std::endl;
      return;
    }
    it=tree->inorderIterator();

    CommonFunctionality::AddItkImageToDataTree<ImageType>(reader->GetOutput(),it,fileName);


    // disable volume rendering by default
//    node->SetProperty("volumerendering",new mitk::BoolProperty(false));

    //mitk::LevelWindow levelWindow;
    //reader->Update();
    //node->SetLevelWindow(levelWindow, NULL);
    //mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);
  }
#endif
  if(it!=NULL)
  {
    mitkMultiWidget->texturizePlaneSubTree(it);
    mitkMultiWidget->updateMitkWidgets();
    mitkMultiWidget->fit();
    delete it;
  }
}

void QmitkMainTemplate::fileOpenImageSequence()
{
  mitk::DataTreeIterator* it=NULL;

  QString fileName = QFileDialog::getOpenFileName(NULL,"DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;stl files (*.stl)");

  if ( !fileName.isNull() )
  {
    int fnstart = fileName.findRev( QRegExp("[/\\\\]"), fileName.length() );
    if(fnstart<0) fnstart=0;
    int start = fileName.find( QRegExp("[0-9]"), fnstart );
    if(start<0)
    {
      fileOpen(fileName.ascii());
      return;
    }

    char prefix[1024], pattern[1024];

    strncpy(prefix, fileName.ascii(), start);
    prefix[start]=0;

    int stop=fileName.find( QRegExp("[^0-9]"), start );
    sprintf(pattern, "%%s%%0%uu%s",stop-start,fileName.ascii()+stop);

    mitk::PicFileReader::Pointer reader;
    reader=mitk::PicFileReader::New();

    std::cout << "loading " << fileName << " interpreted as image sequence mask: prefix " << prefix << " pattern: " << pattern << std::endl;

    reader->SetFilePrefix(prefix);
    reader->SetFilePattern(pattern);

    mitk::DataTreeIterator* it=tree->inorderIterator();

    mitk::DataTreeNode::Pointer node;
    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);
  }

  if(it!=NULL)
  {
    mitkMultiWidget->texturizePlaneSubTree(it);
    mitkMultiWidget->updateMitkWidgets();
    mitkMultiWidget->fit();
    delete it;
  }
}

void QmitkMainTemplate::fileSave()
{

}

void QmitkMainTemplate::fileSaveAs()
{
  mitk::DataTreeIterator* it=tree->inorderIterator()->clone();
  while(it->hasNext())
  {
    it->next();
    mitk::DataTreeNode::Pointer node=it->get();
    if(node->GetData()!=NULL)
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
      if( image.IsNotNull() )
      {
        QString fileName = QFileDialog::getSaveFileName(NULL,"pvtk (*.pvtk)");
        if ( !fileName.isNull() )
        {
          if(fileName.endsWith(".pvtk")==false) fileName+=".pvtk";
          vtkStructuredPointsWriter *writer=vtkStructuredPointsWriter::New();
          writer->SetInput(image->GetVtkImageData());
          writer->SetFileName(fileName.ascii());
          writer->SetFileTypeToBinary();
          writer->Write();
          writer->Delete();
        }
      }
      mitk::SurfaceData::Pointer surface = dynamic_cast<mitk::SurfaceData*>(node->GetData());
      if( surface.IsNotNull() )
      {
        QString fileName = QFileDialog::getSaveFileName(NULL,"stl (*.stl);;vtk (*.vtk)");
        if ( !fileName.isNull() )
        {
          if(fileName.endsWith(".vtk"))
          {
            vtkPolyDataWriter *writer=vtkPolyDataWriter ::New();
            writer->SetInput(surface->GetVtkPolyData());
            writer->SetFileName(fileName.ascii());
            writer->Write();
            writer->Delete();
          }
          else
          {
            if(fileName.endsWith(".stl")==false) fileName+=".stl";
            vtkSTLWriter *writer=vtkSTLWriter ::New();
            writer->SetInput(surface->GetVtkPolyData());
            writer->SetFileName(fileName.ascii());
            writer->Write();
            writer->Delete();
          }
        }
      }
    }
  }

}

void QmitkMainTemplate::filePrint()
{

}

void QmitkMainTemplate::fileExit()
{
  qApp->quit();
}

void QmitkMainTemplate::editUndo()
{
  undoController->Undo(PBShift->isOn());
}

void QmitkMainTemplate::editRedo()
{
  undoController->Redo(PBShift->isOn());
}

void QmitkMainTemplate::editCut()
{

}

void QmitkMainTemplate::editCopy()
{

}

void QmitkMainTemplate::editPaste()
{

}

void QmitkMainTemplate::editFind()
{

}

void QmitkMainTemplate::helpIndex()
{

}

void QmitkMainTemplate::helpContents()
{

}

void QmitkMainTemplate::helpAbout()
{

}


void QmitkMainTemplate::init()
{
  mitkMultiWidget=NULL;

  //creating a QmitkStatusBar for Output on the QStatusBar and connecting it with the MainStatusBar
  QmitkStatusBar::SetStatusBar(this->statusBar());
  //disabling the SizeGrip in the lower right corner
  (QmitkStatusBar::GetInstance())->SetSizeGripEnabled(false);

  //this seems to be a bug of Qt3.1.1's designer: The object name of ToolBar is not initialized.
  ToolBar->setName("ToolBar");

  //create the data tree
  tree=mitk::DataTree::New();
  tree->Register(); //@FIXME: da DataTreeIterator keinen Smartpointer auf DataTree hält, wird tree sonst gelöscht.
}

/*!
\brief basic initialization of main widgets

The method is should be called at the end of the initialize-method of its
subclasses.
*/
void QmitkMainTemplate::initialize()
{
  //initialize interaction sub-system: undo-controller, statemachine-factory and global-interaction

  // test for environment variable MITKCONF
  char* mitkConf = getenv("MITKCONF");
  std::string xmlFileName;

  if (mitkConf != NULL) {
    xmlFileName = mitkConf;
  } else {
    xmlFileName = xmlFallBackPath;
  }
  xmlFileName += "/PointStateMachine.xml";
  if(QFile::exists(xmlFileName.c_str())==false)
    xmlFileName = "PointStateMachine.xml";

  std::cout << "Loading behavior file: " << xmlFileName << std::endl;
  //create undo-controller
  undoController = new mitk::UndoController;

  //create statemachine-factory:
  mitk::StateMachineFactory* stateMachineFactory = new mitk::StateMachineFactory();
  bool smLoadOK = stateMachineFactory->LoadBehavior(xmlFileName);

  //could the behavior file be found?
  if(smLoadOK)
  {
    //create eventmapper-factory:
    mitk::EventMapper* eventMapper = new mitk::EventMapper();
    //Load all possible Events from List
    bool eventLoadOK = eventMapper->LoadBehavior(xmlFileName);
    //could the behavior file be found?
    if(eventLoadOK)
    {
      //set up the global StateMachine and register it to EventMapper
      mitk::GlobalInteraction* globalInteraction = new mitk::GlobalInteraction("global");
      mitk::EventMapper::SetGlobalStateMachine(globalInteraction);
    }
  }
  else
  {
    std::cout<<"Couldn't find XML-configure-file! Check your branch!"<<std::endl;
  }

  //initialize functionality management
  initializeQfm();
  QWidget* defaultMain = qfm->getDefaultMain();

  if(defaultMain!=NULL)
  {
    QBoxLayout *layoutdraw=dynamic_cast<QBoxLayout *>(defaultMain->layout());
    if(layoutdraw==NULL)
      layoutdraw = new QHBoxLayout(defaultMain);

    mitkMultiWidget = new QmitkStdMultiWidget(defaultMain, "QmitkMainTemplate::QmitkStdMultiWidget");
    layoutdraw->addWidget(mitkMultiWidget);

    // add the diplayed planes of the multiwidget to a node to which the subtree @a planesSubTree points ...
    
    mitk::DataTreeIterator* it=tree->inorderIterator();
    mitkMultiWidget->addPlaneSubTree(it);
    mitkMultiWidget->setData(it);
    delete it;

    connect(mitkMultiWidget->levelWindowWidget,SIGNAL(levelWindow(mitk::LevelWindow*)),this,SLOT(changeLevelWindow(mitk::LevelWindow*)) );
  }
  initializeFunctionality();
}


/*!

*/
void QmitkMainTemplate::initializeFunctionality()
{

}

/*!
\brief this method initializes the Qmitk functionality mediator

When subclassing this template class the developer can overwrite the method
to provide different layout templates
*/
void QmitkMainTemplate::initializeQfm()
{
  QHBoxLayout *hlayout;
  hlayout=new QHBoxLayout(MainWidget);

  //create an QmitkFctMediator. This is an invisible object that controls, manages and mediates functionalities
  qfm=new QmitkFctMediator(MainWidget);

  //create an QmitkButtonFctLayoutTemplate. This is an simple example for an layout of the different widgets, of which
  //a functionality and the management consists: the main widget, the control widget and a menu for selecting the
  //active functionality.
  QmitkControlsRightFctLayoutTemplate* layoutTemplate=new QmitkControlsRightFctLayoutTemplate(MainWidget, "LayoutTemplate");
  hlayout->addWidget(layoutTemplate);

  //let the QmitkFctMediator know about the layout. This includes the toolbar and the layoutTemplate.
  qfm->initialize(this);
}

// @FIXME: probably obsolete
// mitk::DataTree* QmitkMainTemplate::getDataTree()
// {
//     return tree;
//}




QmitkStdMultiWidget* QmitkMainTemplate::getMultiWidget()
{
  return mitkMultiWidget;
}

void QmitkMainTemplate::parseCommandLine()
{
  //command line arguments set?
  //The following is a simple method to assess command line arguments.
  int i;
  for(i=1;i<qApp->argc();++i)
  {
    //    const char *param=qApp->argv()[i];
    //    if(param[0]=='-')
    //    {
    //        if(strcmp(param,"-hrm")==0)
    //            hrm=true;
    //        if(strcmp(param,"-number")==0) // e.g., -number 7
    //        {
    //            number= atoi(qApp->argv()[++i]);
    //        }
    //    }
    //    if(qApp->argc()>1)
    fileOpen(qApp->argv()[i]);
  }
}



void QmitkMainTemplate::changeLevelWindow(mitk::LevelWindow* lw )
{

  mitk::DataTreeIterator* it=tree->inorderIterator()->clone();
  while(it->hasNext())
  {
    it->next();

    mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(it->get()->GetPropertyList()->GetProperty("levelwindow").GetPointer());
    if( levWinProp.IsNotNull() )
    {
      mitk::LevelWindow levWin = levWinProp->GetLevelWindow();

      levWin.SetMin(lw->GetMin());
      levWin.SetMax(lw->GetMax());
      levWin.SetRangeMin(lw->GetRangeMin());
      levWin.SetRangeMax(lw->GetRangeMax());

      levWinProp->SetLevelWindow(levWin);

    }
  }
  delete it;
  mitkMultiWidget->updateMitkWidgets();
}


mitk::DataTree::Pointer QmitkMainTemplate::GetTree() {
  return tree;
}


void QmitkMainTemplate::changeTo2DImagesUpLayout()
{
  mitkMultiWidget->changeLayoutTo2DImagesUp();
}
void QmitkMainTemplate::changeTo2DImagesLeftLayout()
{
  mitkMultiWidget->changeLayoutTo2DImagesLeft();
}
void QmitkMainTemplate::changeToDefaultLayout()
{
  mitkMultiWidget->changeLayoutToDefault();
}

void QmitkMainTemplate::changeToBig3DLayout()
{
  mitkMultiWidget->changeLayoutToBig3D();
}

void QmitkMainTemplate::changeToWidget1Layout()
{
  mitkMultiWidget->changeLayoutToWidget1();
}

void QmitkMainTemplate::changeToWidget2Layout()
{
  mitkMultiWidget->changeLayoutToWidget2();
}

void QmitkMainTemplate::changeToWidget3Layout()
{
  mitkMultiWidget->changeLayoutToWidget3();
}

void QmitkMainTemplate::setXMLFallBackPath( const char * anXmlFallBackPath )
{
  xmlFallBackPath = anXmlFallBackPath;
}
