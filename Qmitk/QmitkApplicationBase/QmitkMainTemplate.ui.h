/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "mitkGeometry2DDataVtkMapper3D.h"
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
#include <mitkColorProperty.h>
#include <mitkBoolProperty.h>
#include <mitkLookupTableProperty.h>
#include "mitkProperties.h"

#include <mitkLevelWindowProperty.h>
#include <mitkVesselTreeFileReader.h>
#include <mitkVesselGraphFileReader.h>
#include <mitkLevelWindow.h>
#include <mitkDataTree.h>
#include <mitkFloatProperty.h>
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
#include <mitkDICOMFileReader.h>
#include <mitkDSRFileReader.h>
#include <mitkCylindricToCartesianFilter.h>
#endif

#include <mitkParRecFileReader.h>
#include <mitkInteractionConst.h>
#include <QmitkStatusBar/QmitkStatusBar.h>

mitk::FloatProperty::Pointer opacityprop=NULL;

void QmitkMainTemplate::fileNew()
{

}

void QmitkMainTemplate::fileOpen()
{
#ifdef MBI_INTERNAL
  QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.par *.dcm hpsonos.db HPSONOS.DB);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm)");
#else
  QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.par);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg);;par/rec files (*.par)");
#endif

  if ( !fileName.isNull() )
  {
    fileOpen(fileName.ascii());
    mitkMultiWidget->updateMitkWidgets();
  }
}

void QmitkMainTemplate::fileOpen( const char * fileName )
{
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

      mitk::DataTreeIterator* it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      node->SetData(surface);
      it->add(node);

      initWidgets(it);

      delete it;
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

      mitk::DataTreeIterator* it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      node->SetData(surface);
      it->add(node);

      initWidgets(it);

      delete it;
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

      mitk::DataTreeIterator* it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      node->SetData(reader->GetOutput());
      it->add(node);

      QString tmpName = fileName;
      tmpName = tmpName.right(tmpName.length() - tmpName.findRev("/") - 1);
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(tmpName.ascii());
      node->SetProperty("fileName",nameProp);

      // disable volume rendering by default
      node->SetProperty("volumerendering",new mitk::BoolProperty(false));

      mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
      mitk::LevelWindow levelWindow;
      reader->UpdateLargestPossibleRegion();
      levelWindow.SetAuto( reader->GetOutput()->GetPic() );
      levWinProp->SetLevelWindow(levelWindow);
      node->GetPropertyList()->SetProperty("levelwindow",levWinProp);

      mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);

      initWidgets(it);
    }
  }
  else if(strstr(fileName, ".par")!=0)
  {

    mitk::ParRecFileReader::Pointer reader;

    reader=mitk::ParRecFileReader::New();
    std::cout << "loading " << fileName << " as par/rec ... " << std::endl;
    reader->SetFileName(fileName);

    mitk::DataTreeIterator* it=tree->inorderIterator();

    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);

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
    initWidgets(it);
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

      mitk::DataTreeIterator* it=tree->inorderIterator();

      node=mitk::DataTreeNode::New();
      node->SetData(image);
      it->add(node);

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
      initWidgets(it);
    }
  }
  else if( strstr(fileName, ".ves")!=0 )
  {
    mitk::VesselTreeFileReader::Pointer reader = mitk::VesselTreeFileReader::New();
    std::cout << "loading " << fileName << " as ves ... " << std::endl;
    reader->SetFileName(fileName);
    reader->Update();
    mitk::DataTreeIterator* it=tree->inorderIterator();

    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);

    initWidgets(it);
    delete it;
  }
  else if( strstr(fileName, ".uvg")!=0 )
  {
	  mitk::VesselGraphFileReader::Pointer reader = mitk::VesselGraphFileReader::New();
	  std::cout << "loading " << fileName << " as uvg ... " << std::endl;
	  reader->SetFileName(fileName);
	  reader->Update();
	  mitk::DataTreeIterator* it=tree->inorderIterator();

	  node=mitk::DataTreeNode::New();
	  node->SetData(reader->GetOutput());
	  it->add(node);

	  initWidgets(it);
	  delete it;
  }
#ifdef MBI_INTERNAL
  else if(strstr(fileName, "DCM")!=0)
  {
    mitk::DICOMFileReader::Pointer reader;

    reader=mitk::DICOMFileReader::New();
    std::cout << "loading " << fileName << " as pic ... " << std::endl;

    reader->SetFileName(fileName);

    mitk::DataTreeIterator* it=tree->inorderIterator();

    node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node);

    mitk::LevelWindow levelWindow;
    reader->Update();
    levelWindow.SetAuto( reader->GetOutput()->GetPic() );
    node->SetLevelWindow(levelWindow, NULL);
    mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);

    initWidgets(it);

    delete it;
  }
  else if(strstr(fileName, "hpsonos.db")!=0 || strstr(fileName, "HPSONOS.DB")!=0)
  {

    if (strstr(fileName, "HPSONOS.DB")!=0)
    {
      std::cout << "renamed HPSONOS.DB to hpsonos.db" << std::endl;
      std::string newFileName(fileName);

      int length = newFileName.length();
      newFileName.replace(length-10,10,"hpsonos.db");
      std::cout << "new filename: " << newFileName << std::endl;
      fileName = newFileName.c_str();
    }


    mitk::DSRFileReader::Pointer reader;

    reader=mitk::DSRFileReader::New();
    std::cout << "loading " << fileName << " as DSR ... " << std::endl;

    reader->SetFileName(fileName);
    
    mitk::ImageTimeSelector::Pointer timeSelector=mitk::ImageTimeSelector::New();
    mitk::ImageTimeSelector::Pointer DopplerTimeSelector=mitk::ImageTimeSelector::New();


    mitk::CylindricToCartesianFilter::Pointer cyl2cart = mitk::CylindricToCartesianFilter::New();;
    mitk::CylindricToCartesianFilter::Pointer cyl2cartDoppler = mitk::CylindricToCartesianFilter::New();

		cyl2cart->SetTargetXSize(128);
		cyl2cartDoppler->SetTargetXSize(128); 
		//
    // insert just timeslice nr.0
    // @TODO: insert complete 4D data
    reader->Update();

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

				
    timeSelector->SetInput(reader->GetOutput());
    timeSelector->SetTimeNr(0);

   
    DopplerTimeSelector->SetInput(reader->GetOutput());
    DopplerTimeSelector->SetTimeNr(0);


    mitk::DataTreeIterator* it=tree->inorderIterator();


    //
    // switch to Backscatter information
    //    
    timeSelector->SetChannelNr(0);
    timeSelector->Update();
    ipPicPut("timeselect.pic",timeSelector->GetOutput()->GetPic());    
        
    //
    // insert original (in cylinric coordinates) Backscatter information
    //
    node = mitk::DataTreeNode::New();
    node->SetData(timeSelector->GetOutput());
    mitk::StringProperty::Pointer ultrasoundProp = new mitk::StringProperty("OriginalBackscatter");    
    node->SetProperty("ultrasound",ultrasoundProp);
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty("OriginalBackscatter");
    node->SetProperty("fileName",nameProp);
    node->SetProperty("layer", new mitk::IntProperty(-11) );
    mitk::LevelWindow levelWindow;
    levelWindow.SetAuto( timeSelector->GetOutput()->GetPic() );
    node->SetLevelWindow(levelWindow, NULL);
    node->SetVisibility(false,NULL);
    node->Update();
    it->add(node);    

    //
    // insert transformed (in cartesian coordinates) Backscatter information
    //
    cyl2cart->SetInput(timeSelector->GetOutput());
    node=mitk::DataTreeNode::New();
    node->SetData(cyl2cart->GetOutput());
    ultrasoundProp = new mitk::StringProperty("TransformedBackscatter");
    node->SetProperty("ultrasound",ultrasoundProp);
    nameProp = new mitk::StringProperty("TransformedBackscatter");
    node->SetProperty("fileName",nameProp);
    node->SetProperty("layer", new mitk::IntProperty(-10) );
//    mitk::LevelWindow levelWindow;
    cyl2cart->Update();
    levelWindow.SetAuto( cyl2cart->GetOutput()->GetPic() );
    node->SetLevelWindow(levelWindow, NULL);
    node->Update();
    it->add(node);    

    if (haveDoppler)
    {
				
	    //
  	  // switch to Doppler information
	    //
  	  DopplerTimeSelector->SetChannelNr(1);
	    DopplerTimeSelector->Update();

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
	 	  node->SetData(DopplerTimeSelector->GetOutput());
	    ultrasoundProp = new mitk::StringProperty("OriginalDoppler");
  	  node->SetProperty("ultrasound",ultrasoundProp);
	    nameProp = new mitk::StringProperty("OriginalDoppler");
	    node->SetProperty("fileName",nameProp);
	    node->SetProperty("layer", new mitk::IntProperty(-6) );
//    levelWindow.SetAuto( timeSelector->GetOutput()->GetPic() );
//    node->SetLevelWindow(levelWindow, NULL);
//    node->SetProperty("LookupTable", LookupTableProp );
	    node->SetProperty("LookupTable", LookupTableProp );
  	  node->SetVisibility(false,NULL);
	    node->Update();
  	  it->add(node);

			//
  	  // insert transformed (in cartesian coordinates) Doppler information
    	//
	    cyl2cartDoppler->SetInput(DopplerTimeSelector->GetOutput());
  	  node=mitk::DataTreeNode::New();
	    node->SetData(cyl2cartDoppler->GetOutput());
  	  ultrasoundProp = new mitk::StringProperty("TransformedDoppler");
	    node->SetProperty("ultrasound",ultrasoundProp);
	    nameProp = new mitk::StringProperty("TransformedDoppler");
  	  node->SetProperty("fileName",nameProp);
	    node->SetProperty("layer", new mitk::IntProperty(-5) );
  	  cyl2cartDoppler->Update();
//    levelWindow.SetAuto( cyl2cartDoppler->GetOutput()->GetPic() );
//    node->SetLevelWindow(levelWindow, NULL);
    	node->SetProperty("LookupTable", LookupTableProp );
	    node->Update();
  	  it->add(node);
		}

		
   
    mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);

    initWidgets(it);

    delete it;
  }
#endif
}

void QmitkMainTemplate::fileOpenImageSequence()
{
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

    initWidgets(it);

    mitkMultiWidget->updateMitkWidgets();

    delete it;
  }
}

void QmitkMainTemplate::initWidget(mitk::DataTreeIterator* it,
                                   QmitkSelectableGLWidget* widget,
                                   const Vector3f& normal, const Vector3f& right )
{

  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();

  //ohne den Pointer-Umweg meckert gcc
  mitk::PlaneView* view = new mitk::PlaneView(Plane<float>(Vector3f(0,0,0),normal),
    right,
    Vector2f(0,0),
    Vector2f(0,0)
    );
  plane->SetPlaneView(*view);
  delete view;

  widget->GetRenderer()->SetData(it);
  widget->GetRenderer()->SetWorldGeometry(plane);
  widget->GetRenderer()->GetDisplayGeometry()->Fit();

  //mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  //node->SetData(widget->GetRenderer()->GetWorldGeometry2DData());
  //it->add(node);

  widget->update();
}

void QmitkMainTemplate::initWidget(mitk::DataTreeIterator* it,
                                   QmitkSelectableGLWidget* widget,
                                   const Vector3f& origin,
                                   const Vector3f& right,
                                   const Vector3f& bottom)
{

  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();

  //ohne den Pointer-Umweg meckert gcc
  mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom);
  plane->SetPlaneView(*view);
  delete view;

  widget->GetRenderer()->SetData(it);
  widget->GetRenderer()->SetWorldGeometry(plane);
  widget->GetRenderer()->GetDisplayGeometry()->Fit();

  widget->update();
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

  //initialize interaction sub-system: undo-controller, statemachine-factory and global-interaction

  // test for environment variable MITKCONF
  char* mitkConf = getenv("MITKCONF");
  std::string xmlFileName;

  if (mitkConf != NULL) {
    xmlFileName = mitkConf;
  } else {
    xmlFileName = "../../Interactions/mitkBaseInteraction";
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

  //this seems to be a bug of Qt3.1.1's designer: The object name of ToolBar is not initialized.
  ToolBar->setName("ToolBar");

  //create the data tree
  tree=mitk::DataTree::New();
  tree->Register(); //@FIXME: da DataTreeIterator keinen Smartpointer auf DataTree hält, wird tree sonst gelöscht.
  //create root of data tree (empty)
  //mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  //tree->setRoot(node);
}

/*!
\brief basic initialization of main widgets

The method is should be called at the end of the initialize-method of its
subclasses.
*/
void QmitkMainTemplate::initialize()
{
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
    mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
    it->add(node);
    it->next();
    planesSubTree=dynamic_cast<mitk::DataTreeBase*>(it->getSubTree());

    float white[3] = {1.0f,1.0f,1.0f};
    mitk::DataTreeNode::Pointer planeNode;
    // ... of widget 1
    planeNode=mitkMultiWidget->mitkWidget1->GetRenderer()->GetWorldGeometryNode();
    planeNode->SetColor(white, mitkMultiWidget->mitkWidget4->GetRenderer());
    planeNode->SetProperty("fileName", new mitk::StringProperty("widget1Plane"));
    it->add(planeNode);
    // ... of widget 2
    planeNode=mitkMultiWidget->mitkWidget2->GetRenderer()->GetWorldGeometryNode();
    planeNode->SetColor(white, mitkMultiWidget->mitkWidget4->GetRenderer());
    planeNode->SetProperty("fileName", new mitk::StringProperty("widget2Plane"));
    it->add(planeNode);
    // ... of widget 3
    planeNode=mitkMultiWidget->mitkWidget3->GetRenderer()->GetWorldGeometryNode();
    planeNode->SetColor(white, mitkMultiWidget->mitkWidget4->GetRenderer());
    planeNode->SetProperty("fileName", new mitk::StringProperty("widget3Plane"));
    it->add(planeNode);

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


void QmitkMainTemplate::initWidgets( mitk::DataTreeIterator * it )
{

  mitk::DataTreeIterator *git = planesSubTree->inorderIterator();
  while(git->hasNext())
  {
    git->next();
    if(dynamic_cast<mitk::Geometry2DData*>(git->get()->GetData())!=NULL)
    {
      mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper;
      if(git->get()->GetMapper(2)==NULL)
      {
        geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
        it->get()->SetMapper(2, geometryMapper);
      }
      else
        geometryMapper = dynamic_cast<mitk::Geometry2DDataVtkMapper3D*>(git->get()->GetMapper(2));
      if(geometryMapper.IsNotNull())
        geometryMapper->SetDataIteratorForTexture(it);
    }
  }

  const mitk::BoundingBox::BoundsArrayType bounds = mitk::DataTree::ComputeBoundingBox(it)->GetBounds();
  printf("\nboundingbox\n");

  /*            initWidget(it,
  mitkMultiWidget->mitkWidget1,
  Vector3f(bounds[0],bounds[2],bounds[4]),
  Vector3f(bounds[1],bounds[2],bounds[4]),
  Vector3f(bounds[0],bounds[3],bounds[4])
  );*/
  initWidget(it,
    mitkMultiWidget->mitkWidget1,
    Vector3f(bounds[0],bounds[3],bounds[5]),
    Vector3f(bounds[1],bounds[3],bounds[5]),
    Vector3f(bounds[0],bounds[2],bounds[5])
    );
  printf("\nw1 init\n");

  // YZ
  initWidget(it,mitkMultiWidget->mitkWidget2,
    Vector3f(bounds[0],bounds[2],bounds[4]),
    Vector3f(bounds[0],bounds[3],bounds[4]),
    Vector3f(bounds[0],bounds[2],bounds[5])
    );
  // XZ
  initWidget(it,
    mitkMultiWidget->mitkWidget3,
    Vector3f(bounds[0],bounds[2],bounds[4]),
    Vector3f(bounds[1],bounds[2],bounds[4]),
    Vector3f(bounds[0],bounds[2],bounds[5])
    );
    mitkMultiWidget->mitkWidget4->GetRenderer()->SetData(it);
}



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
