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
#include "QmitkLevelWindowWidget.h"

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
#include <mitkOperation.h>
#include <mitkGlobalInteraction.h>
#include <mitkUSLookupTableSource.h>
#include <mitkConfig.h>

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
#include <QmitkStatusBar.h>


#include <ipPicTypeMultiplex.h>
#include <mitkDataTreeHelper.h>
#include <mitkPointOperation.h>
#include <mitkCoordinateSupplier.h>
#include <mitkStatusBar.h>

template <class T>
static void __buildstring( ipPicDescriptor *pic, itk::Point<int, 3> p, QString &s, T dummy=0)
{
  QString value;
  if ( p[0] < pic->n[0] && p[1] < pic->n[1] && p[2] < pic->n[2] )
  {
    if(pic->bpe!=24)
    {
      value.setNum(((T*) pic->data)[ p[0] + p[1]*pic->n[0] + p[2]*pic->n[0]*pic->n[1] ]);
    }
    else
    {
      value.setNum(((T*) pic->data)[p[0]*3 + 0 + p[1]*pic->n[0]*3 + p[2]*pic->n[0]*pic->n[1]*3 ]);
      value.setNum(((T*) pic->data)[p[0]*3 + 1 + p[1]*pic->n[0]*3 + p[2]*pic->n[0]*pic->n[1]*3 ]);
      value.setNum(((T*) pic->data)[p[0]*3 + 2 + p[1]*pic->n[0]*3 + p[2]*pic->n[0]*pic->n[1]*3 ]);
    }
    s+=value;
  }
  else
  {
    s+= "point out of data";
  }
}
class posOutputType : public mitk::OperationActor
{
  mitk::DataTreeIteratorClone m_DataTreeIterator;
  mitk::ImageTimeSelector::Pointer m_TimeSelector;
public:

  posOutputType(mitk::DataTreeIteratorBase* iterator)
  {
    m_DataTreeIterator = iterator;
    m_TimeSelector = mitk::ImageTimeSelector::New();
  }

  ~posOutputType()
  {}

  typedef mitk::Operation Operation;
  void ExecuteOperation(Operation* operation) //writing mitk::Operation causes QT-Designer to create a Slot calles Operation*operation) and thus causes errors. Thats why we here have a typedef. //TODO: FIX it!
  {
    mitk::PointOperation* pointoperation = dynamic_cast<mitk::PointOperation*>(operation);

    if ( pointoperation != NULL )
    {
      switch ( operation->GetOperationType() )
      {
      case mitk::OpMOVE:
        {
          int maxLayer = itk::NumericTraits<int>::min();
          mitk::Image* image = NULL;
          mitk::DataTreeIteratorClone it = m_DataTreeIterator;
          while ( !it->IsAtEnd() )
          {
            if ( (it->Get().GetPointer() != NULL) && (it->Get()->GetData() != NULL) && it->Get()->IsVisible(NULL) )
            {
              int layer = 0;
              it->Get()->GetIntProperty("layer", layer);
              if ( layer >= maxLayer )
              {

                if(strcmp(it->Get()->GetData()->GetNameOfClass(),"Image")==0)
                {
                  image = static_cast<mitk::Image*>(it->Get()->GetData());
                  maxLayer = layer;
                }
              }
            }
            ++it;
          }

          QString s;
          mitk::Point3D p = pointoperation->GetPoint();
          mitk::ScalarType time = pointoperation->GetTimeInMS();

          s.sprintf("(%.2f,%.2f,%.2f) [mm]", p[0], p[1], p[2]);
          if(time>mitk::ScalarTypeNumericTraits::min())
          {
            QString tmp;
            tmp.sprintf(" %.2f [ms]", time);
            s+=tmp;
          }


          if ( image )
          {
            m_TimeSelector->SetInput(image);

            const mitk::TimeSlicedGeometry* inputTimeGeometry = image->GetUpdatedTimeSlicedGeometry();

            int timestep=0;
            if(time>mitk::ScalarTypeNumericTraits::min())
              timestep = inputTimeGeometry->MSToTimeStep( time );
            if( inputTimeGeometry->IsValidTime( timestep ) == false )
              return;

            m_TimeSelector->SetTimeNr(timestep);
            m_TimeSelector->UpdateLargestPossibleRegion();

            image = m_TimeSelector->GetOutput();

            image->GetGeometry()->WorldToIndex(pointoperation->GetPoint(), p);

            QString pixel;
            if(time>mitk::ScalarTypeNumericTraits::min())
              pixel.sprintf(" (%.2f,%.2f,%.2f,%u) [pixel] ", p[0], p[1], p[2],timestep);
            else
              pixel.sprintf(" (%.2f,%.2f,%.2f) [pixel] ", p[0], p[1], p[2]);
            s+=pixel;

            ipPicDescriptor* pic = image->GetPic();

            mitk::FillVector3D(p, (int)(p[0]+0.5), (int)(p[1]+0.5), (int)(p[2]+0.5));
            if ( image->GetGeometry()->IsIndexInside(p) )
            {
              itk::Point<int, 3> pi;
              mitk::itk2vtk(p, pi);
              if(pic->bpe!=24)
              {
                ipPicTypeMultiplex2(__buildstring, pic, pi, s);
              }
              else
                __buildstring(pic, pi, s, (unsigned char) 1);
            }
          }
          mitk::StatusBar::DisplayText(s.ascii(), 10000);
          break;
        }
      case mitk::OpNOTHING:
        break;
      default:
        ;
      }
    }
  }
};


QmitkMainTemplate* QmitkMainTemplate::m_Instance = NULL;

void QmitkMainTemplate::fileNew()
{}

void QmitkMainTemplate::fileOpen()
{
#ifdef MBI_INTERNAL
  QStringList fileNames = QFileDialog::getOpenFileNames(CommonFunctionality::GetInternalFileExtensions(), NULL);
#else
  QStringList fileNames = QFileDialog::getOpenFileNames( CommonFunctionality::GetExternalFileExtensions(), NULL );
#endif
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    fileOpen((*it).ascii());
  }
}

void QmitkMainTemplate::fileOpen( const char * fileName )
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  mitk::DataTreePreOrderIterator it(tree);
  try
  {
    factory->SetFileName( fileName );

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    factory->Update();
    fileOpenGetFactoryOutput(*factory.GetPointer());
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
  }

  QApplication::restoreOverrideCursor();
}

void QmitkMainTemplate::fileOpenImageSequence()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,CommonFunctionality::GetInternalFileExtensions());

  if ( !fileName.isNull() )
  {
    mitk::DataTreePreOrderIterator it(tree);

    int fnstart = fileName.findRev( QRegExp("[/\\\\]"), fileName.length() );
    if ( fnstart<0 ) fnstart=0;
    int start = fileName.find( QRegExp("[0-9]"), fnstart );
    if ( start<0 )
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

    try
    {
      factory->SetFilePattern( pattern );
      factory->SetFilePrefix( prefix );

      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      factory->Update();
      fileOpenGetFactoryOutput(*factory.GetPointer());
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open: " << ex );
    }

    QApplication::restoreOverrideCursor();
  }
}

void QmitkMainTemplate::fileOpenGetFactoryOutput( mitk::DataTreeNodeFactory & factory)
{
  mitk::DataTreePreOrderIterator it(tree);
  mitk::Image* image = 0;
  mitk::Image* firstImage = 0;
  bool dataFound = false;

  for ( unsigned int i = 0 ; i < factory.GetNumberOfOutputs( ); ++i )
  {
    mitk::DataTreeNode::Pointer node = factory.GetOutput( i );
    if ( node.IsNotNull( ) )
    {
      image = dynamic_cast<mitk::Image*>(node->GetData());
      if ( image != NULL && firstImage == NULL )
      {
        firstImage = image;
      }
      if ( node->GetData() != NULL )
      {
        dataFound = true;
        it.Add( node );
      }
      mitk::LevelWindowProperty::Pointer lw = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer( ) );
      if ( lw.IsNotNull( ) )
      {
        mitkMultiWidget->levelWindowWidget->setLevelWindow( lw->GetLevelWindow() );
      }
    }
  }
  if ( dataFound ) //assure that we have at least one valid output
  {
    if ( m_StandardViewsInitialized == false )
    {
      m_StandardViewsInitialized = mitkMultiWidget->InitializeStandardViews(&it);

      if (firstImage!=NULL)
        if ( firstImage->GetDimension() == 2 )
        {
          mitkMultiWidget->changeLayoutToWidget1();
        }
    }
    mitkMultiWidget->Repaint();
    mitkMultiWidget->Fit();
  }
} // fileOpenGetFactoryOutput()

void QmitkMainTemplate::fileOpenProject()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,"MITK Project File (*.mitk)");

  if ( !fileName.isNull() )
  {
    try
    {    
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      mitk::DataTreePreOrderIterator it(tree);
      mitk::DataTree::Load( &it, fileName.ascii() );

      m_ProjectFileName = fileName;
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open project: " << ex );
    }
    QApplication::restoreOverrideCursor();
  }
}

void QmitkMainTemplate::fileSaveProjectAs()
{
  QString fileName = QFileDialog::getSaveFileName(NULL,"MITK Project File (*.mitk)");

  if ( !fileName.isNull() )
  {
   try
    {    
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      mitk::DataTreePreOrderIterator it(tree);
      mitk::DataTree::Save( &it, fileName.ascii() );

      m_ProjectFileName = fileName;
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open project: " << ex );
    }
  }
  QApplication::restoreOverrideCursor();
}

void QmitkMainTemplate::fileSave()
{
  QString fileName;

  if ( m_ProjectFileName.length() > 5 )
    fileName = m_ProjectFileName;
  else 
    fileName = fileName = QFileDialog::getSaveFileName(NULL,"MITK Project File (*.mitk)");

  if ( !fileName.isNull() )
  {
   try
    {    
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      mitk::DataTreePreOrderIterator it(tree);
      mitk::DataTree::Save( &it, fileName.ascii() );

      m_ProjectFileName = fileName;
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open project: " << ex );
    }
  }
  QApplication::restoreOverrideCursor();
}

void QmitkMainTemplate::filePrint()
{}

void QmitkMainTemplate::fileExit()
{
  qApp->quit();
}

void QmitkMainTemplate::editCut()
{}

void QmitkMainTemplate::editCopy()
{}

void QmitkMainTemplate::editPaste()
{}

void QmitkMainTemplate::editFind()
{}

void QmitkMainTemplate::helpIndex()
{}

void QmitkMainTemplate::helpContents()
{}

void QmitkMainTemplate::helpAbout()
{}


void QmitkMainTemplate::init()
{
  m_Instance = this;
  mitkMultiWidget=NULL;
  m_StandardViewsInitialized = false;
  m_FineUndoEnabled = true;

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
void QmitkMainTemplate::Initialize()
{
  mitk::DataTreePreOrderIterator it(tree);
  //initialize interaction sub-system: undo-controller, statemachine-factory and global-interaction

  // test for environment variable MITKCONF
  char* mitkConf = getenv("MITKCONF");
  std::string xmlFileName;

  if ( mitkConf != NULL )
  {
    xmlFileName = mitkConf;
  }
  else
  {
    xmlFileName = xmlFallBackPath;
  }
  xmlFileName += "/StateMachine.xml";

  if ( QFile::exists(xmlFileName.c_str())==false )
    xmlFileName = "StateMachine.xml";

  std::cout << "Loading behavior file: " << xmlFileName << std::endl;
  //create undo-controller
  undoController = new mitk::UndoController;

  undoButton->setUndoModel( dynamic_cast<mitk::VerboseLimitedLinearUndo*>(mitk::UndoController::GetCurrentUndoModel()) );
  redoButton->setUndoModel( dynamic_cast<mitk::VerboseLimitedLinearUndo*>(mitk::UndoController::GetCurrentUndoModel()) );

  //create statemachine-factory:
  mitk::StateMachineFactory* stateMachineFactory = new mitk::StateMachineFactory();
  bool smLoadOK = stateMachineFactory->LoadBehavior(xmlFileName);

  //could the behavior file be found?
  if ( smLoadOK )
  {
    //create eventmapper-factory:
    mitk::EventMapper* eventMapper = new mitk::EventMapper();
    //Load all possible Events from List
    bool eventLoadOK = eventMapper->LoadBehavior(xmlFileName);
    //could the behavior file be found?
    if ( eventLoadOK )
    {
      //set up the global StateMachine and register it to EventMapper
      mitk::GlobalInteraction* globalInteraction = new mitk::GlobalInteraction("global");
      mitk::EventMapper::SetGlobalStateMachine(globalInteraction);

      posOutputType* posOutput = new posOutputType(&it);

      globalInteraction->AddListener(new mitk::CoordinateSupplier("navigation", posOutput)); //sends PointOperations
    }
  }
  else
  {
    std::cout<<"Couldn't find XML-configure-file! Check your branch!"<<std::endl;
  }

  //initialize functionality management
  InitializeQfm();
  QWidget* defaultMain = qfm->GetDefaultMain();

  if ( defaultMain!=NULL )
  {
    QBoxLayout *layoutdraw=dynamic_cast<QBoxLayout *>(defaultMain->layout());
    if ( layoutdraw==NULL )
      layoutdraw = new QHBoxLayout(defaultMain);

    mitkMultiWidget = new QmitkStdMultiWidget(defaultMain, "QmitkMainTemplate::QmitkStdMultiWidget");
    layoutdraw->addWidget(mitkMultiWidget);

    // add the diplayed planes of the multiwidget to a node to which the subtree @a planesSubTree points ...

    mitkMultiWidget->SetData(&it);

    mitkMultiWidget->AddDisplayPlaneSubTree(&it);
    mitkMultiWidget->EnableStandardLevelWindow();
  }
  InitializeFunctionality();


  mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
  if(globalInteraction!=NULL)
  {
    globalInteraction->AddListener(mitkMultiWidget->GetMoveAndZoomInteractor() );
    globalInteraction->AddListener(mitkMultiWidget->GetMultiplexUpdateController());
  }
}


/*!

*/
void QmitkMainTemplate::InitializeFunctionality()
{}

/*!
\brief this method initializes the Qmitk functionality mediator

When subclassing this template class the developer can overwrite the method
to provide different layout templates
*/
void QmitkMainTemplate::InitializeQfm()
{
  //create an QmitkFctMediator. This is an invisible object that controls, manages and mediates functionalities
  qfm=new QmitkFctMediator(this);

  //create an QmitkButtonFctLayoutTemplate. This is an simple example for an layout of the different widgets, of which
  //a functionality and the management consists: the main widget, the control widget and a menu for selecting the
  //active functionality.
  QmitkControlsRightFctLayoutTemplate* layoutTemplate=new QmitkControlsRightFctLayoutTemplate(this, "LayoutTemplate");
  setCentralWidget(layoutTemplate);

  //let the QmitkFctMediator know about the layout. This includes the toolbar and the layoutTemplate.
  qfm->Initialize(this);
}

// @FIXME: probably obsolete
// mitk::DataTree* QmitkMainTemplate::getDataTree()
// {
//     return tree;
//}




QmitkStdMultiWidget* QmitkMainTemplate::GetMultiWidget()
{
  return mitkMultiWidget;
}

void QmitkMainTemplate::parseCommandLine()
{
  //command line arguments set?
  //The following is a simple method to assess command line arguments.
  int i;
  for ( i=1;i<qApp->argc();++i )
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

mitk::DataTree::Pointer QmitkMainTemplate::GetTree()
{
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
  if ( fullscreen )
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

QmitkMainTemplate* QmitkMainTemplate::GetInstance()
{
  return m_Instance;
}


QmitkFctMediator* QmitkMainTemplate::GetFctMediator()
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
  if ( on )
    ToolBar->hide();
  else
    ToolBar->show();
}


void QmitkMainTemplate::enableFineUndo( bool enabled )
{
  m_FineUndoEnabled = enabled;
  undoButton->setFineUndo(enabled);
  redoButton->setFineUndo(enabled);
}

#include "QmitkSystemInfo.h"
void QmitkMainTemplate::optionsSystem_InformationAction_activated()
{
  QmitkSystemInfo* systemInfo = new QmitkSystemInfo(this, "QmitkSystemInfo");
  systemInfo->show();
}

bool QmitkMainTemplate::GetStandardViewsInitialized()
{
  return m_StandardViewsInitialized;
}

void QmitkMainTemplate::SetStandardViewsInitialized( bool areInitialized )
{
  m_StandardViewsInitialized = areInitialized;
}


void QmitkMainTemplate::editUndo()
{
  if (undoButton->isEnabled())
    undoButton->doUndoRedoLast(1);
}


void QmitkMainTemplate::editRedo()
{
  if (redoButton->isEnabled())
    redoButton->doUndoRedoLast(1);
}
