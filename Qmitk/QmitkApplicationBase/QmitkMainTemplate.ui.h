/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
#include <mitkSurface.h>

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
#endif

#include "QmitkCommonFunctionality.h"
#include <mitkParRecFileReader.h>
#include <mitkInteractionConst.h>
#include <QmitkStatusBar/QmitkStatusBar.h>

QmitkMainTemplate* QmitkMainTemplate::m_Instance = NULL;

void QmitkMainTemplate::fileNew()
{

}

void QmitkMainTemplate::fileOpen()
{
#ifdef MBI_INTERNAL
//  QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.par *.dcm *.mhd *.png *.tif *.jpg hpsonos.db HPSONOS.DB);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm)");
  QString fileName = QFileDialog::getOpenFileName(NULL,CommonFunctionality::GetInternalFileExtensions());
#else
//  QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.par  *.png *.tif *.jpg);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;par/rec files (*.par);;DICOM files (*.dcm)");
  QString fileName = QFileDialog::getOpenFileName( NULL,CommonFunctionality::GetExternalFileExtensions() );
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
        mitk::DataTreePreOrderIterator it(tree);
        mitk::DataTreeNode::Pointer node = factory->GetOutput( i );
        if ( node.IsNotNull( ) )
        {
            it.Add( node );
            mitk::LevelWindowProperty::Pointer lw = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer( ) );
            if ( lw.IsNotNull( ) )
            {
                mitkMultiWidget->levelWindowWidget->setLevelWindow( lw->GetLevelWindow() );
            }
  //          it->removeChild(it->childPosition(node));
        }
    }
    if (factory->GetOutput()->GetData() != NULL) //assure that we have at least one valid output
    {
      mitkMultiWidget->Repaint();
      mitkMultiWidget->Fit();
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
  }
}

void QmitkMainTemplate::fileOpenImageSequence()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,CommonFunctionality::GetInternalFileExtensions());

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
      mitk::DataTreePreOrderIterator it(tree);
      mitk::DataTreeNode::Pointer node = factory->GetOutput( i );
      if ( node.IsNotNull( ) )
      {
        it.Add( node );
        mitk::LevelWindowProperty::Pointer lw = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer( ) );
        if ( lw.IsNotNull( ) )
        {
          mitkMultiWidget->levelWindowWidget->setLevelWindow( lw->GetLevelWindow() );
        }
      }
    }
    if (factory->GetOutput()->GetData() != NULL) //assure that we have at least one valid output
    {
      mitkMultiWidget->Repaint();
      mitkMultiWidget->Fit();
    }
  }
}

void QmitkMainTemplate::fileSave()
{

}

void QmitkMainTemplate::fileSaveAs()
{
  mitk::DataTreePreOrderIterator it(tree);
  while(!it.IsAtEnd())
  {
    mitk::DataTreeNode::Pointer node=it.Get();
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
      mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(node->GetData());
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
    ++it;
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
  m_Instance = this;  
  mitkMultiWidget=NULL;

  //creating a QmitkStatusBar for Output on the QStatusBar and connecting it with the MainStatusBar
  QmitkStatusBar *statusBar = new QmitkStatusBar(this->statusBar());
  //disabling the SizeGrip in the lower right corner
  statusBar->SetSizeGripEnabled(false);

  //this seems to be a bug of Qt3.1.1's designer: The object name of ToolBar is not initialized.
  ToolBar->setName("ToolBar");

  //create the data tree
  tree=mitk::DataTree::New();
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
    
    mitk::DataTreePreOrderIterator it(tree);

    mitkMultiWidget->SetData(&it);

    mitkMultiWidget->AddDisplayPlaneSubTree(&it);
    mitkMultiWidget->EnableStandardLevelWindow();
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

void QmitkMainTemplate::FullScreenMode(bool fullscreen)
{
  if (fullscreen)
    showFullScreen();
  else
    showNormal();
}

void QmitkMainTemplate::setXMLFallBackPath( const char * anXmlFallBackPath )
{
  xmlFallBackPath = anXmlFallBackPath;
}


void QmitkMainTemplate::m_ShowPlanesCheckBox_clicked()
{
    emit ShowWidgetPlanesToggled( m_ShowPlanesCheckBox->isOn() );
}


void QmitkMainTemplate::destroy()
{
  delete qfm;
}

QmitkMainTemplate* QmitkMainTemplate::getInstance()
{
    return m_Instance;
}


QmitkFctMediator* QmitkMainTemplate::getFctMediator()
{
    return qfm;
}

void QmitkMainTemplate::changeToColumnWidget3n4Layout()
{
    mitkMultiWidget->changeLayoutToColumnWidget3And4();
}


void QmitkMainTemplate::changeToRowWidget3n4Layout()
{
    mitkMultiWidget->changeLayoutToRowWidget3And4();
}


void QmitkMainTemplate::hideToolbar(bool on)
{
  if(on)
    ToolBar->hide();
  else
    ToolBar->show();
}
