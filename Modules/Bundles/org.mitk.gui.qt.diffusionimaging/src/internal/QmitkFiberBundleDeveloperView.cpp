/*=========================================================================
 
 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
 Version:   $Revision: 21975 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>



// Qmitk
#include "QmitkFiberBundleDeveloperView.h"
#include <QmitkStdMultiWidget.h>

// Qt

// VTK
#include <vtkPointSource.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>

//not needed
//#include <vtkPolyDataMapper.h>
//#include <vtkActor.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkRenderWindow.h>


const std::string QmitkFiberBundleDeveloperView::VIEW_ID = "org.mitk.views.fiberbundledeveloper";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;


QmitkFiberBundleDeveloperView::QmitkFiberBundleDeveloperView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
  
}

// Destructor
QmitkFiberBundleDeveloperView::~QmitkFiberBundleDeveloperView()
{
  
}


void QmitkFiberBundleDeveloperView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done in QtDesigner, etc.
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberBundleDeveloperViewControls;
    m_Controls->setupUi( parent );
    
    connect( m_Controls->buttonGenerateFibers, SIGNAL(clicked()), this, SLOT(DoGenerateFibers()) );
    
    
  }
  
  //  Checkpoint for fiber ORIENTATION
  if ( m_DirectionRadios.empty() )
  {
    m_DirectionRadios.insert(0, m_Controls->radioButton_directionRandom);
    m_DirectionRadios.insert(1, m_Controls->radioButton_directionX);
    m_DirectionRadios.insert(2, m_Controls->radioButton_directionY);
    m_DirectionRadios.insert(3, m_Controls->radioButton_directionZ);
  }
  
}

void QmitkFiberBundleDeveloperView::DoGenerateFibers()
{
  
  // GET SELECTED FIBER DIRECTION
  QString fibDirection; //stores the object_name of selected radiobutton 
  QVector<QRadioButton*>::const_iterator i;
  for (i = m_DirectionRadios.begin(); i != m_DirectionRadios.end(); ++i) 
  {
    QRadioButton* rdbtn = *i;
    if (rdbtn->isChecked())
      fibDirection = rdbtn->objectName();
  }
  
  vtkSmartPointer<vtkPolyData> FiberPD; // FiberPD stores the generated PolyData
  if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_RANDOM ) {
    //    build polydata with random lines and fibers
    FiberPD = GenerateVtkFibersRandom();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_X ) {
    //    build polydata with XDirection fibers
    FiberPD = GenerateVtkFibersDirectionX();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_Y ) {
    //    build polydata with YDirection fibers
    FiberPD = GenerateVtkFibersDirectionY();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_Z ) {
    //    build polydata with ZDirection fibers
    FiberPD = GenerateVtkFibersDirectionZ();
    
  }
  
  
  
} 


/*
 * here we are using "dynamic" feeding of cell arrays
 */
vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersRandom()
{
  int numOfFibers = m_Controls->boxFiberNumbers->value();
  int pntsPrFiber = m_Controls->boxPointsPerFiber->value();
  int numOfPoints = numOfFibers * pntsPrFiber;
  
  MITK_INFO << "Numer of Fibers: " << numOfFibers << "\nPoints per Fiber: " << pntsPrFiber << "\nPoints Total: " << numOfPoints; 
  
  
  vtkSmartPointer<vtkPointSource> randomPoints = vtkSmartPointer<vtkPointSource>::New();
  randomPoints->SetCenter(0.0, 0.0, 0.0);
  randomPoints->SetNumberOfPoints(numOfPoints);
  randomPoints->SetRadius(pntsPrFiber);
  randomPoints->Update();

  vtkPolyData* pntHost = randomPoints->GetOutput();
  vtkPoints* pnts = pntHost->GetPoints();
  
  //checkpoint if requested amount of points equals generated amount
  if (numOfPoints != pnts->GetNumberOfPoints()) {
    MITK_INFO << "VTK POINT ERROR, WRONG AMOUNT OF GENRERATED POINTS";
    return 0;
  }  
  
  // Set Fibers
  vtkSmartPointer<vtkCellArray> linesCell = vtkSmartPointer<vtkCellArray>::New();
  
  // iterate through points
  for (int i=0; i < numOfPoints; i+=pntsPrFiber) // e.g. i eqals 0, 50, 100, etc., I choose this cuz then its more easy to fill the lines with according points.
  {
    if (i%pntsPrFiber == 0) 
    {
      vtkSmartPointer<vtkPolyLine> newFiber = vtkSmartPointer<vtkPolyLine>::New();
      newFiber->GetPointIds()->SetNumberOfIds(pntsPrFiber);
    
      //      construct the fiber
      for (int pc = 0; pc < pntsPrFiber; ++pc) 
      {
        newFiber->GetPointIds()->SetId(pc, pc+i);
        MITK_INFO << linesCell->GetNumberOfCells() << ": " << pnts->GetPoint(pc+i)[0] << " " << pnts->GetPoint(pc+i)[1] << " " << pnts->GetPoint(pc+i)[2];
      }
      
      linesCell->InsertNextCell(newFiber);
      
      
      //    ================SHALL NEVER OCCUR=================
    } else {
      MITK_INFO << "LOGIC ERROR IN CREATING FIBERS...Check Values in QmitkFiberBundleDeveloperView.cpp";
    }
    //    ===================================================     
  }
  
  vtkSmartPointer<vtkPolyData> PDRandom = vtkSmartPointer<vtkPolyData>::New();
  
  
    PDRandom->SetPoints(pnts);
    PDRandom->SetLines(linesCell);
  
//  // Setup actor and mapper
//  vtkSmartPointer<vtkPolyDataMapper> mapper = 
//  vtkSmartPointer<vtkPolyDataMapper>::New();
//  mapper->SetInput(polyData);
//  
//  vtkSmartPointer<vtkActor> actor = 
//  vtkSmartPointer<vtkActor>::New();
//  actor->SetMapper(mapper);
//  
//  // Setup render window, renderer, and interactor
//  vtkSmartPointer<vtkRenderer> renderer = 
//  vtkSmartPointer<vtkRenderer>::New();
//  vtkSmartPointer<vtkRenderWindow> renderWindow = 
//  vtkSmartPointer<vtkRenderWindow>::New();
//  renderWindow->AddRenderer(renderer);
//  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
//  vtkSmartPointer<vtkRenderWindowInteractor>::New();
//  renderWindowInteractor->SetRenderWindow(renderWindow);
//  renderer->AddActor(actor);
//  
//  renderWindow->Render();
//  renderWindowInteractor->Start();
//  
  return PDRandom;
}


vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersDirectionX()
{
  vtkSmartPointer<vtkPolyData> PDX = vtkSmartPointer<vtkPolyData>::New();
  //todo
  
  
  
  return PDX;
}

vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersDirectionY()
{
  vtkSmartPointer<vtkPolyData> PDY = vtkSmartPointer<vtkPolyData>::New();
  //todo
  
  
  
  return PDY;
}


vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersDirectionZ()
{
  vtkSmartPointer<vtkPolyData> PDZ = vtkSmartPointer<vtkPolyData>::New();
  //todo
  
  
  
  return PDZ;
}



void QmitkFiberBundleDeveloperView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkFiberBundleDeveloperView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
 implement SelectionService Listener explicitly */

void QmitkFiberBundleDeveloperView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  
  
}

void QmitkFiberBundleDeveloperView::Activated()
{
  
  MITK_INFO << "FB OPerations ACTIVATED()";
  
  
}



