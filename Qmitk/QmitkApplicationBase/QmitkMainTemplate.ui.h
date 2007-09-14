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

/**

  \class QmitkMainTemplate

  Base application that is used by most MITK applications.

  \section QmitkMainTemplateHelpBrowser Help window

  \subsection QmitkMainTemplateHelpBrowserSub1 Which help file is displayed

  QmitkMainTemplate supports a very basic help system. If the users presses F1 or chooses Help/Content from the menu,
  QmitkMainTemplate opens a QmitkHelpBrowser to display a HTML page that should explain the currently active functionality.

  The intelligence behind finding the correct help page is not very sophisticated: from the application options, the value
  of "HTML documentation path" is read. It is expected that there are HTML pages in this directory. To this path is added
  "FUNCTIONALITY_CLASS_NAMEUserManual.html", where FUNCTIONALITY_CLASS_NAME is the class name of a concrete functionality.

  <b>Example</b>: Your functionality class is called "QmitkSimpleExampleFunctionality", and in the application options you chose 
           "HTML documentation path" to point to "~/mitk/doxygen/". The help browser will then try to open
           ~/mitk/doxygen/QmitkSimpleExampleFunctionalityUserManual.html
  

  \subsection QmitkMainTemplateHelpBrowserSub2 How to create the help page

  If you have Doxygen installed on your system, the MITK build system is able to generate a lot of HTML documentation from
  all the classes of the source tree. To do that you have to build the target "doc" (Visual Studio: right-click the target, 
  choose "Build/Erzeugen"; Linux: make doc)

  To add a help page to the Doxygen output, you should create a directory "UserManual" inside your functionality directory 
  (because the build system configures Doxygen to look for images in these directories).
  Inside the directory "UserManual" you create a file called FUNCTIONALITY_CLASS_NAMEUserManual.dox with basically this content 

\verbatim

/**
\page QmitkFUNCTIONALITY_CLASS_NAMEUserManual The functionality to solve all problems

Available sections:
  - \ref QmitkFUNCTIONALITY_CLASS_NAMEUserManualOverview 
  - \ref QmitkFUNCTIONALITY_CLASS_NAMEUserManualDetails 
  - \ref QmitkFUNCTIONALITY_CLASS_NAMEUserManualGrundfunktionen 

\section QmitkLymphNodeVolumetryUserManualOverview Overview of the problems

Bla

\section QmitkFUNCTIONALITY_CLASS_NAMEUserManualDetails Complications in details

Blaa

\section QmitkFUNCTIONALITY_CLASS_NAMEUserManualGrundfunktionen How to solve everything using FUNCTIONALITY_CLASS_NAME

Tadaa

*&#47;
\endverbatim

  However, it is nice to actually replace the nonsense :-)
*/

// PLEASE: do not remove this strange 47 character up ther in the doxygen documentation. Putting a normal slash there produced errors on
// at least one system.

// these files have to be included at this location
// because otherwise microsoft visual c++ generates
// an internal compiler error
#include <mitkConfig.h> //for MBI_INTERNAL
#include <mitkCoreObjectFactory.h>
#ifdef MBI_INTERNAL
  #include <mitkVesselTreeFileReader.h>
  #include <mitkVesselGraphFileReader.h>
  #include <mitkDICOMFileReader.h>
  #include <mitkDSRFileReader.h>
  #include <mitkCylindricToCartesianFilter.h>
  #include <QmitkSaveProjectWidget.h>
  #include <itksys/SystemTools.hxx>
#else
  #include <itkImage.h>
  #include <itkImageFileReader.h>
  #include <itkDICOMImageIO2.h>
  #include <itkImageSeriesReader.h>
  #include <itkDICOMSeriesFileNames.h>
#endif

#include <QmitkCommonFunctionality.h>
#include <QmitkSelectableGLWidget.h>
#include <QmitkHelpBrowser.h>

#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkImageData.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkRenderer.h>

#include <map>

#include <mitkProperties.h>

#include <mitkDataTree.h>
#include <string>

#include <mitkStringProperty.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateOR.h>
#include <mitkNodePredicateNOT.h>

#include <QmitkPropertyListView.h>


#include <mitkImageTimeSelector.h>

#include <mitkStateMachineFactory.h>
#include <mitkOperation.h>
#include <mitkGlobalInteraction.h>
#include <mitkStandardFileLocations.h>
#include <mitkUndoController.h>

#include <mitkParRecFileReader.h>
#include <mitkInteractionConst.h>
#include <QmitkStatusBar.h>
#include <QmitkProgressBar.h>
#include <QmitkMemoryUsageIndicator.h>
#include <QmitkSystemInfo.h>
#include <QmitkRawImageFileOpener.h>
#include <QmitkRawImageFileSequenceOpener.h>

#include <QmitkOptionDialog.h>
#include <qlistbox.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qmetaobject.h>
#include <qregexp.h>
#include <qstring.h>
#include <qgrid.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qmessagebox.h>

#include <ipPicTypeMultiplex.h>
#include <mitkPointOperation.h>
#include <mitkCoordinateSupplier.h>
#include <mitkStatusBar.h>

#include <stdexcept>
#include "mitkChiliPlugin.h"

template <class T>
static void __buildstring( ipPicDescriptor *pic, itk::Point<int, 3> p, QString &s, T /*dummy*/=0)
{
  QString value;
  
  if ( (p[0]>=0 && p[1] >=0 && p[2]>=0) && (unsigned int)p[0] < pic->n[0] && (unsigned int)p[1] < pic->n[1] && (unsigned int)p[2] < pic->n[2] )
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
		if(m_DataTreeIterator.IsNotNull())
		{
			itk::ReceptorMemberCommand<posOutputType>::Pointer command = itk::ReceptorMemberCommand<posOutputType>::New();
			command->SetCallbackFunction(this, &posOutputType::TreeChanged);
			m_ObserverTag = m_DataTreeIterator->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
		}
  }

  ~posOutputType()
  {
		m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
	}

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
          mitk::Image* image = NULL;
					image = GetImageFromDataTree();
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


					if (image==NULL)
					{
						m_TimeSelector->SetInput(NULL);
					}
					else if ( image->IsInitialized() ) 
          {
            m_TimeSelector->SetInput(image);
            mitk::Image* image3D = m_TimeSelector->GetOutput();

            const mitk::TimeSlicedGeometry* inputTimeGeometry = image->GetUpdatedTimeSlicedGeometry();

            int timestep=0;
            if(time>mitk::ScalarTypeNumericTraits::min())
              timestep = inputTimeGeometry->MSToTimeStep( time );
            if ( ( inputTimeGeometry->IsValidTime( timestep ) == false ) || ( image->IsVolumeSet(timestep) == false ) )
            {
              if(image3D!=NULL)
              {
                image3D->ReleaseData();
              }
              //m_TimeSelector->SetInput(NULL);
              return;
            }

            m_TimeSelector->SetTimeNr(timestep);
            m_TimeSelector->UpdateLargestPossibleRegion();

            image3D->GetGeometry()->WorldToIndex(pointoperation->GetPoint(), p);

            QString pixel;
            if(time>mitk::ScalarTypeNumericTraits::min())
              pixel.sprintf(" (%.2f,%.2f,%.2f,%u) [pixel] ", p[0], p[1], p[2],timestep);
            else
              pixel.sprintf(" (%.2f,%.2f,%.2f) [pixel] ", p[0], p[1], p[2]);
            s+=pixel;

            ipPicDescriptor* pic = image3D->GetPic();
            if ( pic )
            {

              mitk::FillVector3D(p, (int)(p[0]+0.5), (int)(p[1]+0.5), (int)(p[2]+0.5));
              if ( image3D->GetGeometry()->IsIndexInside(p) )
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

            if(image3D!=NULL)
            {
              image3D->ReleaseData();
            }
            //m_TimeSelector->SetInput(NULL);
          }
          mitk::StatusBar::GetInstance()->DisplayText(s.ascii(), 10000);
          break;
        }
      case mitk::OpNOTHING:
        break;
      default:
        ;
      }
    }
  }

  virtual void TreeChanged(const itk::EventObject & treeChangedEvent)
  {
    mitk::Image* image = NULL;
    image = this->GetImageFromDataTree();
    if (image==NULL)
    {
      m_TimeSelector->SetInput(NULL);
    }
    else if ( image->IsInitialized() )
    {
      m_TimeSelector->SetInput(image);
    }
  }

  mitk::Image* GetImageFromDataTree()
  {
    int maxLayer = itk::NumericTraits<int>::min();
    mitk::Image* image = NULL;
    mitk::DataTreeIteratorClone it = m_DataTreeIterator;
    while ( !it->IsAtEnd() )
    {
      bool include = true;
      if(it->Get()->GetBoolProperty("include for pixel inspection", include) == false)
        include = it->Get()->IsVisible(NULL);
      if ( (it->Get().GetPointer() != NULL) && (it->Get()->GetData() != NULL) && include )
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
    return image;
  }

  int m_ObserverTag;
};

QmitkMainTemplate* QmitkMainTemplate::m_Instance = NULL;

#ifdef MBI_INTERNAL
  QmitkSaveProjectWidget* m_SceneWidget;
#endif

void QmitkMainTemplate::fileOpen()
{
  try
  {
    QString defaultPath;
    if ( m_FirstFileOpen )
    {
      defaultPath = m_DefaultDatasetPath->GetValue();
    }
    else
    {
      defaultPath = QString::null;
    }
    
    QStringList fileNames = QFileDialog::getOpenFileNames(
      mitk::CoreObjectFactory::GetInstance()->GetFileExtensions(), defaultPath);

    if ( !fileNames.isEmpty() )
    {
      QStringList::Iterator it;
      for ( it = fileNames.begin(); it != fileNames.end(); ++it )
      {
        fileOpen((*it).ascii());
      }
      m_FirstFileOpen = false;
    }
  }
  catch ( std::exception& e )
  {
    itkGenericOutputMacro( << "Exception caught: " << e.what() );
    QMessageBox::critical ( this, "Exception caught!", e.what() );
  }
}

void QmitkMainTemplate::fileOpen( const char * fileName )
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  mitk::DataTreePreOrderIterator it(m_Tree);
  try
  {
    factory->SetFileName( fileName );
    factory->SetImageSerie(false);

    /*QString qFileName( fileName );
    
    // just in case this is a series
    int fnstart = qFileName.findRev( QRegExp("[/\\\\]"), qFileName.length() ); // last occurence of / or \  (\\\\ because of C++ quoting and regex syntax)
    if ( fnstart<0 ) fnstart=0;
    int start = qFileName.find( QRegExp("[0-9]*\\."), fnstart );
    if ( start>=0 )
    {
      char prefix[1024], pattern[1024];

      strncpy(prefix, qFileName.ascii(), start);
      prefix[start]=0;

      int stop=qFileName.find( QRegExp("[^0-9]"), start );
      sprintf(pattern, "%%s%%0%uu%s",stop-start,qFileName.ascii()+stop);

      if (start != stop)
      {
        factory->SetImageSerie(true);
        factory->SetFilePattern( pattern );
        factory->SetFilePrefix( prefix );
      }
    }*/

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    factory->Update();
    fileOpenGetFactoryOutput(*factory.GetPointer());
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
    QMessageBox::critical ( this, "Exception caught!", ex.what() );
  }

  QApplication::restoreOverrideCursor();
}

void QmitkMainTemplate::fileOpenImageSequence()
{
  try
  {
    QString defaultPath;
    if ( m_FirstFileOpen )
    {
      defaultPath = m_DefaultDatasetPath->GetValue();
    }
    else
    {
      defaultPath = QString::null;
    }
    
    QString fileName = QFileDialog::getOpenFileName(defaultPath,mitk::CoreObjectFactory::GetInstance()->GetFileExtensions(), 0, 0, "Open Sequence");

    if ( !fileName.isNull() )
    {
      mitk::DataTreePreOrderIterator it(m_Tree);

      std::string path = itksys::SystemTools::GetFilenamePath(fileName.ascii());
      std::string name = itksys::SystemTools::GetFilenameName(fileName.ascii());

      QString nameq = name.c_str();
      int start = nameq.find( QRegExp("[0-9]{1,}\\.") );
      if ( start<0 )
      {
        fileOpen(fileName.ascii());
        return;
      }

      char prefix[1024], pattern[1024];
      // now we want to work with fileName again to include the path, thus
      // add the length of the path to start; the variable "path" does not 
      // contain trailing slashes, therefore the second addend in the followong line:
      start += path.length()+(fileName.length()-path.length()-name.length());

      strncpy(prefix, fileName.ascii(), start);
      prefix[start]=0;

      int stop=fileName.find( QRegExp("[^0-9]"), start );
      sprintf(pattern, "%%s%%0%uu%s",stop-start,fileName.ascii()+stop);


      mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
      try
      {
        factory->SetFileName(fileName.ascii());
        factory->SetImageSerie(false);
        if (start != stop)
        {
          factory->SetImageSerie(true);
          factory->SetFilePattern( pattern );
          factory->SetFilePrefix( prefix );
        }
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        factory->Update();
        factory->SetImageSerie(false);
        fileOpenGetFactoryOutput(*factory.GetPointer());
        m_FirstFileOpen = false;
      }
      catch ( itk::ExceptionObject & ex )
      {
        itkGenericOutputMacro( << "Exception during file open: " << ex );
      }

      QApplication::restoreOverrideCursor();
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical ( this, "Exception caught!", e.what() );
  }
}

void QmitkMainTemplate::fileOpenGetFactoryOutput( mitk::DataTreeNodeFactory & factory)
{
  mitk::DataTreePreOrderIterator it(m_Tree);
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
        mitk::DataStorage::GetInstance()->Add(node);
      }
    }
  }
  
  m_StandardViewsInitialized = m_MultiWidget->InitializeStandardViews(&it);
  m_MultiWidget->RequestUpdate();
  m_MultiWidget->Fit();
  m_StandardViewsInitialized = true;
  
  if ( dataFound ) //assure that we have at least one valid output
  {
    if (firstImage!=NULL)
      if ( firstImage->GetDimension(2) == 1 )
      {
        m_MultiWidget->changeLayoutToWidget1();
      }
  }
} 

void QmitkMainTemplate::fileOpenProject()
{
#ifdef MBI_INTERNAL
  QString filename = QFileDialog::getOpenFileName( QString::null, "XML Project description (*.xml)",0,
                                                   "Open Project File", "Choose a file to open");
  if ( !filename.isEmpty() ) {
    try
    {
      mitk::DataTreePreOrderIterator it(m_Tree);
      mitk::DataTree::Load(&it, filename);
      m_MultiWidget->InitializeStandardViews(&it);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open project: " << ex );
    }
  } 
#else
  QString fileName = QFileDialog::getOpenFileName(NULL,"MITK Project File (*.mitk)");

  if ( !fileName.isNull() )
  {
    try
    {    
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      mitk::DataTreePreOrderIterator it(m_Tree);
      mitk::DataTree::Load( &it, fileName.ascii() );
      m_MultiWidget->InitializeStandardViews(&it);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      m_ProjectFileName = fileName;
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open project: " << ex );
    }
    QApplication::restoreOverrideCursor();
  }
#endif
}

void QmitkMainTemplate::fileSaveProjectAs()
{
#ifdef MBI_INTERNAL
  try
  {
    m_SceneWidget = new QmitkSaveProjectWidget(m_Tree, 0);
    m_SceneWidget->show();
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open project: " << ex );
  }
#else
  QString fileName = QFileDialog::getSaveFileName(NULL,"MITK Project File (*.mitk)");

  if ( !fileName.isNull() )
  {
  try
    {    
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      mitk::DataTreePreOrderIterator it(m_Tree);
      mitk::DataTree::Save( &it, fileName.ascii() );

      m_ProjectFileName = fileName;
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open project: " << ex );
    }
  }
  QApplication::restoreOverrideCursor();
#endif
}

void QmitkMainTemplate::fileSave()
{
#ifdef MBI_INTERNAL
  try
  {
    m_SceneWidget = new QmitkSaveProjectWidget(m_Tree, 0);
    m_SceneWidget->show();
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open project: " << ex );
  }
#else  
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

      mitk::DataTreePreOrderIterator it(m_Tree);
      mitk::DataTree::Save( &it, fileName.ascii() );

      m_ProjectFileName = fileName;
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open project: " << ex );
    }
  }
  QApplication::restoreOverrideCursor();
#endif
}

void QmitkMainTemplate::fileExit()
{
  qApp->quit();
}

void QmitkMainTemplate::init()
{
  redoButton->setMode(QUndoRedoButton::Redo);
  m_Instance = this;
  m_MultiWidget=NULL;
  m_StandardViewsInitialized = false;
  m_FineUndoEnabled = true;
  m_StandardPalette = QApplication::palette();
  m_FirstFileOpen = true;

  //creating a QmitkStatusBar for Output on the QStatusBar and connecting it with the MainStatusBar
  QmitkStatusBar *statusBar = new QmitkStatusBar(this->statusBar());
  //disabling the SizeGrip in the lower right corner
  statusBar->SetSizeGripEnabled(false);

  QmitkProgressBar *progBar = new QmitkProgressBar();
  this->statusBar()->addWidget(progBar, 0, true);
  progBar->hide();

  QmitkMemoryUsageIndicator* memoryIndicator = new QmitkMemoryUsageIndicator();
  this->statusBar()->addWidget(memoryIndicator, 0, true);
  
  //this seems to be a bug of Qt3.1.1's designer: The object name of ToolBar is not initialized.
  ToolBar->setName("ToolBar");

  //create the data m_Tree
  m_Tree=mitk::DataTree::New();

  // create the DataStorage
  mitk::DataStorage::CreateInstance(m_Tree);

  m_Options = mitk::PropertyList::New();
  m_Options->SetProperty( "Use gradient background", new mitk::BoolProperty(true) );
  m_Options->SetProperty( "Gradient color 1", new mitk::ColorProperty( 0.0f, 0.1f, 0.3f ) );
  m_Options->SetProperty( "Gradient color 2", new mitk::ColorProperty( 0.7f, 0.7f, 0.8f ) );
  m_Options->SetProperty( "Background color", new mitk::ColorProperty(0.0f,0.0f,0.0f) );
  m_Options->SetProperty( "HTML documentation path", new mitk::StringProperty("/local/ip++bin/Documentations/Doxygen/html/") );
  m_Options->SetProperty( "Use dark palette", new mitk::BoolProperty(false) );
  m_Options->SetProperty( "Department logo visible", new mitk::BoolProperty(false) );
  m_Options->SetProperty( "Department logo path", new mitk::StringProperty("") );
  m_Options->SetProperty( "Default value for iilInterpolation", new mitk::BoolProperty(mitk::DataTreeNodeFactory::m_IilInterpolationActive) );
  m_Options->SetProperty( "Default dataset path", new mitk::StringProperty("") );
}

/*!
\brief basic initialization of main widgets

The method is should be called at the end of the initialize-method of its
subclasses.
*/
void QmitkMainTemplate::Initialize()
{
  mitk::DataTreePreOrderIterator it(m_Tree);
  //initialize interaction sub-system: undo-controller, statemachine-factory and global-interaction

  //create undo-controller
  m_UndoController = new mitk::UndoController;

  undoButton->setUndoModel( dynamic_cast<mitk::VerboseLimitedLinearUndo*>(mitk::UndoController::GetCurrentUndoModel()) );
  redoButton->setUndoModel( dynamic_cast<mitk::VerboseLimitedLinearUndo*>(mitk::UndoController::GetCurrentUndoModel()) );

  //create global-interaction
  std::cout << "Loading behavior file: " << std::flush;
  bool smLoadOK = mitk::GlobalInteraction::StandardInteractionSetup();
  std::cout << mitk::StateMachineFactory::GetLastLoadedBehavior() << std::endl;

  //could the behavior file be found?
  if ( smLoadOK )
  {
    posOutputType* posOutput = new posOutputType(&it);

    mitk::GlobalInteraction::GetInstance()->AddListener(mitk::CoordinateSupplier::New("navigation", posOutput)); //sends PointOperations
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

    m_MultiWidget = new QmitkStdMultiWidget(defaultMain, "QmitkMainTemplate::QmitkStdMultiWidget");
    layoutdraw->addWidget(m_MultiWidget);

    // add the diplayed planes of the multiwidget to a node to which the subtree @a planesSubTree points ...

    m_MultiWidget->SetData(&it);

    m_MultiWidget->AddDisplayPlaneSubTree(&it);
    m_MultiWidget->AddPositionTrackingPointSet(&it); //mouse position
    m_MultiWidget->EnableStandardLevelWindow();

    // show/hide plane widgets when the corresponding buttons/menu items are checked
    connect(toolbarShowPlanes,    SIGNAL(toggled(bool)), m_MultiWidget, SLOT(SetWidgetPlanesVisibility(bool)));
    connect(viewShowPlanesAction, SIGNAL(toggled(bool)), m_MultiWidget, SLOT(SetWidgetPlanesVisibility(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesVisibilityChanged(bool)), toolbarShowPlanes, SLOT(setOn(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesVisibilityChanged(bool)), viewShowPlanesAction, SLOT(setOn(bool)));

    // lock/unlock plane widgets when the corresponding buttons/menu items are checked... 
    connect(toolbarLockPlanes,    SIGNAL(toggled(bool)), m_MultiWidget, SLOT(SetWidgetPlanesLocked(bool)));
    connect(viewLockPlanesAction, SIGNAL(toggled(bool)), m_MultiWidget, SLOT(SetWidgetPlanesLocked(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesLockedChanged(bool)), toolbarLockPlanes, SLOT(setOn(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesLockedChanged(bool)), viewLockPlanesAction, SLOT(setOn(bool)));

    // lock/unlock rotations of plane widgets when the corresponding buttons/menu items are checked...  (menu only)
    connect(viewLockSliceRotationAction, SIGNAL(toggled(bool)), m_MultiWidget, SLOT(SetWidgetPlanesRotationLocked(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesRotationLockedChanged(bool)), viewLockSliceRotationAction, SLOT(setOn(bool)));

    // linking of slices during rotation
    connect(viewLinkSliceRotationAction, SIGNAL(toggled(bool)), m_MultiWidget, SLOT(SetWidgetPlanesRotationLinked(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesRotationLinked(bool)), viewLinkSliceRotationAction, SLOT(setOn(bool)));

    
    // enabling/disabling of slices rotation in general
    connect(viewSlicesRotationAction, SIGNAL(toggled(bool)), m_MultiWidget, SLOT(SetWidgetPlanesRotationEnabled(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesRotationEnabled(bool)), viewSlicesRotationAction, SLOT(setOn(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesRotationEnabled(bool)), viewLinkSliceRotationAction, SLOT(setEnabled(bool)));
    connect(m_MultiWidget, SIGNAL(WidgetPlanesRotationEnabled(bool)), viewLockSliceRotationAction, SLOT(setEnabled(bool)));

   }

  this->InitializeFunctionality();


  std::string optionsFile(mitk::StandardFileLocations::GetInstance()->FindFile("MITKOptions.xml"));

  if (!optionsFile.empty())
  {
    LoadOptionsFromFile(optionsFile.c_str());
  }
  m_Options->SetProperty( "MITKSampleAppFunctionalityName", new mitk::StringProperty("MITKSampleApp") );

  // Pass global options to all available dialog bars (other than
  // functionalities, dialog bars currently store their options
  // (enabled/disabled state and potentially other options in the
  // global options list).
  qfm->ApplyOptionsToDialogBars( m_Options );

  // initialize multiwidget with options
  // gradient background
  mitk::BoolProperty* gradProperty = dynamic_cast<mitk::BoolProperty*>( m_Options->GetProperty("Use gradient background").GetPointer() );          
  if (gradProperty != NULL)
    this->enableGradientBackground(gradProperty->GetValue());

  mitk::ColorProperty* upperColProp = dynamic_cast<mitk::ColorProperty*>( m_Options->GetProperty("Gradient color 1").GetPointer() );
  mitk::ColorProperty* lowerColProp = dynamic_cast<mitk::ColorProperty*>( m_Options->GetProperty("Gradient color 2").GetPointer() );
  if  ( upperColProp && lowerColProp )
  {
    m_MultiWidget->SetGradientBackgroundColors( upperColProp->GetColor(), lowerColProp->GetColor() );
  }
  // department logo
  mitk::BoolProperty* logoProperty = dynamic_cast<mitk::BoolProperty*>( m_Options->GetProperty("Department logo visible").GetPointer() );          
  if (logoProperty != NULL)
    this->enableDepartmentLogo(logoProperty->GetValue());
  mitk::StringProperty::Pointer logoPath = dynamic_cast<mitk::StringProperty*>( m_Options->GetProperty("Department logo path").GetPointer() );
  if (logoPath)
  {
    m_MultiWidget->SetDepartmentLogoPath(logoPath->GetValue());
  }

  // default dataset path
  m_DefaultDatasetPath = dynamic_cast<mitk::StringProperty*>( 
    m_Options->GetProperty("Default dataset path").GetPointer() );

  // use dark palette on/off
  mitk::BoolProperty* darkProperty = dynamic_cast<mitk::BoolProperty*>( m_Options->GetProperty("Use dark palette").GetPointer() );          
  if(mitk::ChiliPlugin::GetInstance()->IsPlugin())
    this->enableDarkPalette(true);
  else
    if (darkProperty != NULL)
      this->enableDarkPalette(darkProperty->GetValue());

  mitk::ColorProperty* colProperty = dynamic_cast<mitk::ColorProperty*>( m_Options->GetProperty("Background color").GetPointer() );
  mitk::Color c = colProperty->GetColor();
  m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->SetBackground(c.GetRed(), c.GetGreen(), c.GetBlue());

  // Initialize other global options
  mitk::BoolProperty* iilProperty = dynamic_cast<mitk::BoolProperty*>( m_Options->GetProperty("Default value for iilInterpolation").GetPointer() );          
  if (iilProperty != NULL)
    mitk::DataTreeNodeFactory::m_IilInterpolationActive = iilProperty->GetValue();

  // Add MoveAndZoomInteractor and widget NavigationControllers as
  // GlobalInteraction listeners
  mitk::GlobalInteraction::GetInstance()->AddListener(
    m_MultiWidget->GetMoveAndZoomInteractor()
    );

  m_MultiWidget->EnableNavigationControllerEventListening();
}


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
  qfm->Initialize( this );
  
}



QmitkStdMultiWidget* QmitkMainTemplate::GetMultiWidget()
{
  return m_MultiWidget;
}

void QmitkMainTemplate::parseCommandLine()
{
  if(strstr(qApp->argv()[0], "chili") != NULL)
    return;
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
    if(strcmp(qApp->argv()[i], "-testing")==0) {
      break;
    }
    if(strcmp(qApp->argv()[i], "-testdata")==0) {
      mitk::DataTreePreOrderIterator it(m_Tree);
      mitk::DataTreeNode::Pointer treeNode = mitk::DataTreeNode::New();
      treeNode->SetData(mitk::Image::New());
      it.Add(treeNode);
      break;
    }
    fileOpen(qApp->argv()[i]);
  }
}

mitk::DataTree::Pointer QmitkMainTemplate::GetTree()
{
  return m_Tree;
}

void QmitkMainTemplate::changeTo2DImagesUpLayout()
{
  m_MultiWidget->changeLayoutTo2DImagesUp();
}

void QmitkMainTemplate::changeTo2DImagesLeftLayout()
{
  m_MultiWidget->changeLayoutTo2DImagesLeft();
}

void QmitkMainTemplate::changeToDefaultLayout()
{
  m_MultiWidget->changeLayoutToDefault();
}

void QmitkMainTemplate::changeToBig3DLayout()
{
  m_MultiWidget->changeLayoutToBig3D();
}

void QmitkMainTemplate::changeToWidget1Layout()
{
  m_MultiWidget->changeLayoutToWidget1();
}

void QmitkMainTemplate::changeToWidget2Layout()
{
  m_MultiWidget->changeLayoutToWidget2();
}

void QmitkMainTemplate::changeToWidget3Layout()
{
  m_MultiWidget->changeLayoutToWidget3();
}

void QmitkMainTemplate::changeToColumnWidget3n4Layout()
{
  m_MultiWidget->changeLayoutToColumnWidget3And4();
}

void QmitkMainTemplate::changeToRowWidget3n4Layout()
{
  m_MultiWidget->changeLayoutToRowWidget3And4();
}

void QmitkMainTemplate::changeTo2x2Dand3DLayout()
{
  m_MultiWidget->changeLayoutTo2x2Dand3DWidget();
}

void QmitkMainTemplate::changeToLeft2DAnd3DRight2DLayout()
{
  m_MultiWidget->changeLayoutToLeft2Dand3DRight2D();
}


void QmitkMainTemplate::FullScreenMode(bool fullscreen)
{
  if ( fullscreen )
    showFullScreen();
  else
    showNormal();
}

void QmitkMainTemplate::destroy()
{
  std::string filename( mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() );
  filename += "/MITKOptions.xml";
  SaveOptionsToFile( filename.c_str() );
  
  delete qfm;
#ifdef MBI_INTERNAL
  delete m_SceneWidget;
#endif
}

QmitkMainTemplate* QmitkMainTemplate::GetInstance()
{
  return m_Instance;
}

QmitkFctMediator* QmitkMainTemplate::GetFctMediator()
{
  return qfm;
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

void QmitkMainTemplate::viewReinitMultiWidget()
{
  mitk::DataTreePreOrderIterator it(m_Tree);
  m_MultiWidget->InitializeStandardViews( &it );
}

void QmitkMainTemplate::helpContents()
{
  mitk::BaseProperty::Pointer bp = m_Options->GetProperty("HTML documentation path");
  mitk::StringProperty* pathproperty = dynamic_cast<mitk::StringProperty*>( bp.GetPointer() );
  
  QDir homedir( pathproperty->GetValueAsString().c_str() );
  QString home( homedir.absPath() + "/" );
  QString firstpage = home;

  if (qfm)
  {
    // try to find documentation of the active functionality
    QmitkFunctionality* f = qfm->GetActiveFunctionality();
    if (f)
    {
      firstpage += f->metaObject()->className(); 
      firstpage += "UserManual.html";
    }

    // fallback to SampleApp index page
    if (!QFile::exists( firstpage ))
    {
      firstpage = home + "MITKSampleApp";
      firstpage += "UserManual.html";

    }
  }
  QmitkHelpBrowser* browser = new QmitkHelpBrowser( firstpage, ".", NULL, "Online help");
  browser->setCaption("MITK documentation");
  browser->showMaximized();
}

void QmitkMainTemplate::optionsShow_OptionsAction_activated()
{
  QmitkOptionDialog* optionDialog = new QmitkOptionDialog(this, "Options");
  
  // first add a global options panel
  optionDialog->m_FunctionalitySelectionList->insertItem("Global options", 1);  // start at index 1, because index 0 does not show up in gui

  // use a PropertyListView widget to display global options
  QmitkPropertyListView* dialog = new QmitkPropertyListView(this);
  dialog->SetPropertyList(m_Options);
  optionDialog->m_OptionWidgetStack->addWidget(dialog, 1);
  
  // for each functionality: If the funcionality has an option widget, 
  // add it to the  m_FunctionalitySelectionList and the m_OptionWidgetStack
  for (unsigned int i = 0; i < qfm->GetFunctionalityCount(); ++i)
  {
    QmitkFunctionality* f = qfm->GetFunctionalityById(i);
    optionDialog->m_FunctionalitySelectionList->insertItem(f->GetFunctionalityName(), i+2);
    QWidget* optionWidget = f->CreateOptionWidget(this);
    if (optionWidget == NULL)
      optionWidget = new QLabel("no options available", this);
    optionDialog->m_OptionWidgetStack->addWidget(optionWidget, i+2);
  }
  
  // preselect active functionality
  optionDialog->m_FunctionalitySelectionList->setSelected(qfm->GetActiveFunctionalityId() + 1, true);
  optionDialog->m_OptionWidgetStack->raiseWidget(qfm->GetActiveFunctionalityId() + 2);

  // show the dialog
  if (optionDialog->exec() == QDialog::Accepted)
  {
    // first process global options
    
    // gradient background
    mitk::BaseProperty::Pointer bp =  m_Options->GetProperty("Use gradient background");
    mitk::BoolProperty* gradProperty = dynamic_cast<mitk::BoolProperty*>( bp.GetPointer() );
    this->enableGradientBackground(gradProperty->GetValue());

    mitk::ColorProperty* upperColProp = dynamic_cast<mitk::ColorProperty*>( m_Options->GetProperty("Gradient color 1").GetPointer() );
    mitk::ColorProperty* lowerColProp = dynamic_cast<mitk::ColorProperty*>( m_Options->GetProperty("Gradient color 2").GetPointer() );
    if  ( upperColProp && lowerColProp )
    {
      m_MultiWidget->SetGradientBackgroundColors( upperColProp->GetColor(), lowerColProp->GetColor() );
    }
    
    // department logo
    mitk::BaseProperty::Pointer logop =  m_Options->GetProperty("Department logo visible");
    mitk::BoolProperty* logoProperty = dynamic_cast<mitk::BoolProperty*>( logop.GetPointer() );
    mitk::StringProperty::Pointer logoPath = dynamic_cast<mitk::StringProperty*>( m_Options->GetProperty("Department logo path").GetPointer() );
    if (logoPath)
    {
      m_MultiWidget->SetDepartmentLogoPath(logoPath->GetValue());
    }
    this->enableDepartmentLogo(logoProperty->GetValue());
    
    // default dataset path
    m_DefaultDatasetPath = dynamic_cast<mitk::StringProperty*>( 
    m_Options->GetProperty("Default dataset path").GetPointer() );

    // dark palette
    mitk::BoolProperty* darkProperty = dynamic_cast<mitk::BoolProperty*>( m_Options->GetProperty("Use dark palette").GetPointer() );          
    if(mitk::ChiliPlugin::GetInstance()->IsPlugin())
      this->enableDarkPalette(true);
    else
      if (darkProperty != NULL)
        this->enableDarkPalette(darkProperty->GetValue());

    bp =  m_Options->GetProperty("Background color");
    mitk::ColorProperty* colProperty = dynamic_cast<mitk::ColorProperty*>( bp.GetPointer() );
    mitk::Color c = colProperty->GetColor();
    m_MultiWidget->setBackgroundColor(QColor((int)c.GetRed(),(int)c.GetGreen(), (int)c.GetBlue()));
    m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer()->SetBackground(c.GetRed(), c.GetGreen(), c.GetBlue());

    mitk::BoolProperty* iilProperty = dynamic_cast<mitk::BoolProperty*>( m_Options->GetProperty("Default value for iilInterpolation").GetPointer() );          
    if (iilProperty != NULL)
      mitk::DataTreeNodeFactory::m_IilInterpolationActive = iilProperty->GetValue();

    // Pass global options to all available dialog bars (other than
    // functionalities, dialog bars currently store their options
    // (enabled/disabled state and potentially other options in the global
    // options list).
    qfm->ApplyOptionsToDialogBars( m_Options );
    
    // next, notify the functionalities of changes in their options
    for (unsigned int i = 0; i < qfm->GetFunctionalityCount(); ++i)
    {
      QmitkFunctionality* f = qfm->GetFunctionalityById(i);
      if (f != NULL)
        f->OptionsChanged(optionDialog->m_OptionWidgetStack->widget(i + 2));
    } 
  }
  delete optionDialog;
}

void QmitkMainTemplate::SaveOptionsToFile(const char* filename)
{
  //create a XMLWriter
  mitk::XMLWriter xmlw(filename);
  // start tree
  xmlw.BeginNode(mitk::DataTree::XML_NODE_NAME);

  // write SampleApp's options
  xmlw.BeginNode(mitk::DataTree::XML_TAG_TREE_NODE);
  xmlw.BeginNode(mitk::DataTreeNode::XML_NODE_NAME);
  xmlw.WriteProperty(mitk::XMLIO::CLASS_NAME, "DataTreeNode");
    xmlw.BeginNode(mitk::PropertyList::XML_NODE_NAME);
    xmlw.WriteProperty(mitk::XMLIO::CLASS_NAME, "PropertyList");
      m_Options->WriteXMLData( xmlw );
    xmlw.EndNode();
  xmlw.EndNode();
  xmlw.EndNode();

  // write each functionalities options
  for (unsigned int i = 0; i < qfm->GetFunctionalityCount(); ++i)
  {
    QmitkFunctionality* f = qfm->GetFunctionalityById(i);
    mitk::PropertyList* fo = f->GetFunctionalityOptionsList();
    if (fo && !fo->IsEmpty())
    {
      fo->SetProperty( "MITKSampleAppFunctionalityName", new mitk::StringProperty( f->GetFunctionalityName().ascii() ) );

      xmlw.BeginNode(mitk::DataTree::XML_TAG_TREE_NODE);
      xmlw.BeginNode(mitk::DataTreeNode::XML_NODE_NAME);
      xmlw.WriteProperty(mitk::XMLIO::CLASS_NAME, "DataTreeNode");
        xmlw.BeginNode(mitk::PropertyList::XML_NODE_NAME);
        xmlw.WriteProperty(mitk::XMLIO::CLASS_NAME, "PropertyList");
          fo->WriteXMLData( xmlw );
        xmlw.EndNode();
      xmlw.EndNode();
      xmlw.EndNode();
    }

  }
 
  // end tree
  xmlw.EndNode();
}

void QmitkMainTemplate::LoadOptionsFromFile(const char* filename)
{
  // create a dummy tree with all the functionalities' propertylists
  mitk::DataTree::Pointer dummyTree = mitk::DataTree::New();

  mitk::DataTreePreOrderIterator iter(dummyTree);
  mitk::DataTree::Load( &iter, filename );

  // traverse the tree, tell the appropriate functionalities about their loaded options
  iter.GoToBegin();
 
  while (!iter.IsAtEnd())
  {
    // get propertylist
    // look for property "MITKSampleAppFunctionalityName"
    // if this is "MITKSampleApp", then take it as global options
    // else find the belonging functionality, ask it to take that list
    mitk::DataTreeNode* node = iter.Get();
    if (node)
    {
      mitk::PropertyList* pl = node->GetPropertyList();
      if (pl)
      {
        mitk::BaseProperty::Pointer bp = pl->GetProperty("MITKSampleAppFunctionalityName");
        mitk::StringProperty* id = dynamic_cast<mitk::StringProperty*>( bp.GetPointer() );
        std::string idstring;
        if (id) 
          idstring = id->GetValueAsString();
        
        if (idstring == "MITKSampleApp")
        {
          // set all global options read from the config file
          mitk::PropertyList::PropertyMap::const_iterator it;
          for ( it = pl->GetMap()->begin(); it != pl->GetMap()->end(); it++ )
          {
            m_Options->SetProperty(it->first.c_str(), it->second.first);
          }
        }
        else
        {
          // give it to the appropriate functionality
          QmitkFunctionality* f = qfm->GetFunctionalityByName( idstring.c_str() );
          if ( f != NULL )
          {
            f->SetFunctionalityOptionsList( pl );
          }
        }
      }
    } 
    
    ++iter;
  }
}

void QmitkMainTemplate::viewShowPlanesAction_toggled( bool on )
{
  if( on )
  {
    toolbarShowPlanes->setText("Hide Planes");
    toolbarShowPlanes->setMenuText("Hide Planes");
    toolbarShowPlanes->setToolTip("Hide Planes");  
  }
  else    
  {
    toolbarShowPlanes->setText("Show Planes");
    toolbarShowPlanes->setMenuText("Show Planes");
    toolbarShowPlanes->setToolTip("Show Planes"); 
  }

  toolbarShowPlanes->setOn( on );
  viewShowPlanesAction->setOn( on );
}



void QmitkMainTemplate::toolbarPositionOrientation_toggled( bool on )
{
  //@brief Setting Input-Device (Mouse) Tracking - for Projection in other widgets
  if(on)
    m_MultiWidget->EnablePositionTracking();
  else
    m_MultiWidget->DisablePositionTracking();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkMainTemplate::changeToRowWidgetSmall3nBig4Layout()
{
  m_MultiWidget->changeLayoutToRowWidgetSmall3andBig4();
}


void QmitkMainTemplate::changeToSmallUpperWidget2Big3n4Layout()
{
    m_MultiWidget->changeLayoutToSmallUpperWidget2Big3and4();
}

// New code for Raw Image reading
void QmitkMainTemplate::fileOpenRawImage()
{
    QStringList fileNames = QFileDialog::getOpenFileName("/home","Raw images: (*.raw *.ct)", NULL);
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    fileOpenRawImage((*it).ascii());
  }
}

//new code for Raw Image reading
void QmitkMainTemplate::fileOpenRawImage( const char * fileName )
{
 
  if(!fileName) return;

  // open dialog window and get parameters
  QmitkRawImageFileOpener* rawFileOpener = new QmitkRawImageFileOpener(this, "QmitkRawFileOpener");
  
  mitk::Image::Pointer m_ResultImage = rawFileOpener->ShowAndTryToRead(fileName);
  
  if (m_ResultImage.IsNotNull())
  {
      mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
      node->SetData(m_ResultImage);
      mitk::DataTreeNodeFactory::SetDefaultImageProperties(node);
      node->SetProperty("name", new mitk::StringProperty( fileName ));
      mitk::DataStorage::GetInstance()->Add(node);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      mitk::DataTreePreOrderIterator it(m_Tree);
      m_MultiWidget->InitializeStandardViews(&it);  // otherwise it is not seen
  }
  
}

// code for Raw Image Sequence Reading (multiple 2D slices belonging to the same 3D stack) 
void QmitkMainTemplate::fileOpenRawImageSequence()
{
  QStringList fileNames = QFileDialog::getOpenFileNames("Raw images: (*.raw *.ct)","/home", NULL);
  fileOpenRawImageSequence(fileNames);
  
}

void QmitkMainTemplate::fileOpenRawImageSequence(QStringList fileNames)
{
  if(!fileNames.empty())
  {    
    // open dialog window and get parameters
    QmitkRawImageFileSequenceOpener* rawFileSequenceOpener = new QmitkRawImageFileSequenceOpener(this, "QmitkRawFileSequenceOpener");
  
    mitk::Image::Pointer m_ResultImage = rawFileSequenceOpener->ShowAndTryToRead(fileNames);
  
    if (m_ResultImage.IsNotNull())
    {
        mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
        node->SetData(m_ResultImage);
        mitk::DataTreeNodeFactory::SetDefaultImageProperties(node);
        node->SetProperty("name", new mitk::StringProperty( fileNames.first().ascii() ));
        mitk::DataStorage::GetInstance()->Add(node);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        mitk::DataTreePreOrderIterator it(m_Tree);
        m_MultiWidget->InitializeStandardViews(&it);  // otherwise it is not seen
    }
  }
    
}

void QmitkMainTemplate::enableGradientBackground( bool enable)
{
  if(enable)
  {
    m_MultiWidget->EnableGradientBackground();
  }
  else
  {
    m_MultiWidget->DisableGradientBackground();
  }
}

void QmitkMainTemplate::enableDarkPalette( bool enable )
{
  if(enable)
  {
    this->m_StandardPalette = QApplication::palette();
    QPalette p( QColor( 64,64,64), QColor(64,64,64));
    QApplication::setPalette(p,TRUE);
  }
  else
  {
    QApplication::setPalette(this->m_StandardPalette,TRUE);
  }
}

void QmitkMainTemplate::fileCloseProject()
{
  /* This method deletes all nodes but the three widget planes, their grouping node 
     and nodes that do not have a data object. This should free most of the used memory.
     If other nodes (with helper objects like the widget planes) should be kept, they have
     to be added here as a predicate. */
  mitk::NodePredicateProperty w1("name", new mitk::StringProperty("Widgets"));        // keep helper objects
  mitk::NodePredicateProperty w2("name", new mitk::StringProperty("widget1Plane"));
  mitk::NodePredicateProperty w3("name", new mitk::StringProperty("widget2Plane"));
  mitk::NodePredicateProperty w4("name", new mitk::StringProperty("widget3Plane"));
  mitk::NodePredicateData w5(NULL);   // keep objects without data (e.g. root node of the tree!)
  mitk::NodePredicateOR orpred;
  orpred.AddPredicate(w1);
  orpred.AddPredicate(w2);
  orpred.AddPredicate(w3);
  orpred.AddPredicate(w4);
  orpred.AddPredicate(w5);
  mitk::NodePredicateNOT notpred(orpred);
  mitk::DataStorage::SetOfObjects::ConstPointer all = mitk::DataStorage::GetInstance()->GetSubset(notpred);
  mitk::DataStorage::GetInstance()->Remove(all);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  if (m_UndoController)
  {
    m_UndoController->Clear(); // clear the undo/redo stacks
  }
}


void QmitkMainTemplate::enableDepartmentLogo(bool enable)
{
  if(enable)
  {
    m_MultiWidget->EnableDepartmentLogo();
  }
  else
  {
    m_MultiWidget->DisableDepartmentLogo();
  }

}
