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
#include <mitkProperties.h>
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
#include <mitkDataTreeNodeFactory.h>
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
  QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.par);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;par/rec files (*.par);;DICOM files (*.dcm)");
#endif

  if ( !fileName.isNull() )
  {
    fileOpen(fileName.ascii());
  }
}

void QmitkMainTemplate::fileOpen( const char * fileName )
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
  
  try
  {
    factory->SetFileName( fileName );
    factory->Update();

    for (unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i)
    {
        mitk::DataTreeIterator* it = tree->inorderIterator( );
        mitk::DataTreeNode::Pointer node = factory->GetOutput( i );
        if ( node.IsNotNull( ) )
        {
            it->add( node );
            mitk::LevelWindowProperty::Pointer lw = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer( ) );
            if ( lw.IsNotNull( ) )
            {
                mitkMultiWidget->levelWindowWidget->setLevelWindow( lw->GetLevelWindow() );
            }
  //          it->removeChild(it->childPosition(node));
        }
        delete it;
    }
    if (factory->GetOutput()->GetData() != NULL) //assure that we have at least one valid output
    {
      mitk::DataTreeIterator* it = tree->inorderIterator();
      mitkMultiWidget->texturizePlaneSubTree( tree->inorderIterator());
      mitkMultiWidget->updateMitkWidgets();
      mitkMultiWidget->fit();
      delete it;
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
  }
}

void QmitkMainTemplate::fileOpenImageSequence()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,"DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;Sets of 2D slices (*.pic *.pic.gz *.png *.dcm);;stl files (*.stl);;DICOM files(*.dcm *.DCM)");

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


    mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
  
    factory->SetFilePattern( pattern );
    factory->SetFilePrefix( prefix );
    factory->Update();

    for (unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i)
    {
      mitk::DataTreeIterator* it = tree->inorderIterator( );
      mitk::DataTreeNode::Pointer node = factory->GetOutput( i );
      if ( node.IsNotNull( ) )
      {
        it->add( node );
        mitk::LevelWindowProperty::Pointer lw = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer( ) );
        if ( lw.IsNotNull( ) )
        {
          mitkMultiWidget->levelWindowWidget->setLevelWindow( lw->GetLevelWindow() );
        }
      }
      delete it;
    }
    if (factory->GetOutput()->GetData() != NULL) //assure that we have at least one valid output
    {
      mitk::DataTreeIterator* it = tree->inorderIterator();
      mitkMultiWidget->texturizePlaneSubTree(it);
      mitkMultiWidget->updateMitkWidgets();
      mitkMultiWidget->fit();
      delete it;
    }
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
  QmitkStatusBar *statusBar = new QmitkStatusBar(this->statusBar());
  //disabling the SizeGrip in the lower right corner
  statusBar->SetSizeGripEnabled(false);

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
  xmlFileName += "/StateMachine.xml";

  if(QFile::exists(xmlFileName.c_str())==false)
    xmlFileName = "StateMachine.xml";

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
