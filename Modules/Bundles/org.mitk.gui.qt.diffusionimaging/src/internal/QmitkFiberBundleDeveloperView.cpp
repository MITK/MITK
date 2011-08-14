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
  
  vtkSmartPointer<vtkPolyData> output; // FiberPD stores the generated PolyData
  if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_RANDOM ) {
    //    build polydata with random lines and fibers
    output = GenerateVtkFibersRandom();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_X ) {
    //    build polydata with XDirection fibers
    output = GenerateVtkFibersDirectionX();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_Y ) {
    //    build polydata with YDirection fibers
    output = GenerateVtkFibersDirectionY();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_Z ) {
    //    build polydata with ZDirection fibers
    output = GenerateVtkFibersDirectionZ();
    
  }
  
  //mitkFiberBundleX::New()
  
  
} 


/*
 * Generate polydata of random fibers
 */
vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersRandom()
{
  int numOfFibers = m_Controls->boxFiberNumbers->value();
  int pntsPrFiber = m_Controls->boxPointsPerFiber->value();
  int numOfPoints = numOfFibers * pntsPrFiber;
  
  //  MITK_INFO << "Numer of Fibers: " << numOfFibers << "\nPoints per Fiber: " << pntsPrFiber << "\nPoints Total: " << numOfPoints; 
  
  vtkSmartPointer<vtkPointSource> randomPoints = vtkSmartPointer<vtkPointSource>::New();
  randomPoints->SetCenter(0.0, 0.0, 0.0);
  randomPoints->SetNumberOfPoints(numOfPoints);
  randomPoints->SetRadius(pntsPrFiber);
  randomPoints->Update();
  
  vtkPolyData* pntHost = randomPoints->GetOutput();
  vtkPoints* pnts = pntHost->GetPoints();
  
  //===================================
  //checkpoint if requested amount of points equals generated amount
  if (numOfPoints != pnts->GetNumberOfPoints()) {
    MITK_INFO << "VTK POINT ERROR, WRONG AMOUNT OF GENRERATED POINTS";
    return 0;
  }// ================================= 
  
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
//        MITK_INFO << linesCell->GetNumberOfCells() << ": " << pnts->GetPoint(pc+i)[0] << " " << pnts->GetPoint(pc+i)[1] << " " << pnts->GetPoint(pc+i)[2];
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
  
  
  return PDRandom;
}


vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersDirectionX()
{
  int numOfFibers = m_Controls->boxFiberNumbers->value();
  int pntsPrFiber = m_Controls->boxPointsPerFiber->value();
  int numOfPoints = numOfFibers * pntsPrFiber;
  
  vtkSmartPointer<vtkCellArray> linesCell = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  
  //insert Origin point, this point has index 0 in point array
  double originX = 0.0;
  double originY = 0.0;
  double originZ = 0.0;
  
  //after each iteration the origin of the new fiber increases
  //here you set which direction is affected
  double increaseX = 0.0;
  double increaseY = 1.0;
  double increaseZ = 0.0;
  
  //walk along X axis
  //length of fibers increases in each iteration
  for (int i=0; i<numOfFibers; ++i) {
    
    //insert origin point of new fiber
    points->InsertNextPoint(originX + (double)i * increaseX , originY + (double)i * increaseY, originZ + (double)i * increaseZ);
    
    MITK_INFO << i << ": OP " << points->GetPoint(points->GetNumberOfPoints()-1)[0] << " " << points->GetPoint(points->GetNumberOfPoints()-1)[1] << " " << points->GetPoint(points->GetNumberOfPoints()-1)[2];
    
    
    //insert remaining points for fiber
    for (int pj=0; pj<=i ; ++pj) 
    { //generate next point
      points->InsertNextPoint( originX + (double)pj+1 * increaseX , originY + (double)pj+1 * increaseY, originZ + (double)pj+1 * increaseZ );

      
    }
        
    vtkSmartPointer<vtkPolyLine> newFiber = vtkSmartPointer<vtkPolyLine>::New();
    newFiber->GetPointIds()->SetNumberOfIds(i+2);
    newFiber->GetPointIds()->SetId(0,0);
    
    
    linesCell->InsertNextCell(newFiber);
  }
  
  
  vtkSmartPointer<vtkPolyData> PDX = vtkSmartPointer<vtkPolyData>::New(); 
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



