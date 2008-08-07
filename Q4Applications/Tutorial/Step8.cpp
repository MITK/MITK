#include "Step8.h"

#include "QmitkStdMultiWidget.h"

#include "mitkGlobalInteraction.h"
#include "mitkRenderingManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

//##Documentation
//## @brief As Step6, but with QmitkStdMultiWidget as widget
Step8::Step8( int argc, char* argv[], QWidget *parent )
  : Step6( argc, argv, parent )
{
}

void Step8::SetupWidgets()
{
  //*************************************************************************
  // Part I: Create windows and pass the tree to it
  //*************************************************************************

  // Create toplevel widget with vertical layout
  QVBoxLayout* vlayout = new QVBoxLayout(this);
  vlayout->setMargin(0);
  vlayout->setSpacing(2);

  // Create viewParent widget with horizontal layout
  QWidget* viewParent = new QWidget(this);
  vlayout->addWidget(viewParent);
  QHBoxLayout* hlayout = new QHBoxLayout(viewParent);
  hlayout->setMargin(0);

  //*************************************************************************
  // Part Ia: create and initialize QmitkStdMultiWidget
  //*************************************************************************
  QmitkStdMultiWidget* multiWidget = new QmitkStdMultiWidget(viewParent);
  hlayout->addWidget(multiWidget);
  
  // Create an iterator on the tree
  mitk::DataTreePreOrderIterator it(m_Tree);

  // Tell the multiWidget which (part of) the tree to render
  // This will be changed to take a DataStorage object
  multiWidget->SetData(&it);

  // Initialize views as transversal, sagittal, coronar (from
  // top-left to bottom)
  mitk::RenderingManager::GetInstance()->InitializeViews(mitk::DataStorage::GetInstance());

  // Initialize bottom-right view as 3D view
  multiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(
    mitk::BaseRenderer::Standard3D );

  // Enable standard handler for levelwindow-slider
  multiWidget->EnableStandardLevelWindow();

  // Add the displayed views to the tree to see their positions
  // in 2D and 3D
  // This will be changed to take a DataStorage object
  multiWidget->AddDisplayPlaneSubTree(&it);

  //*************************************************************************
  // Part II: Setup standard interaction with the mouse
  //*************************************************************************

  // Moving the cut-planes to click-point
  multiWidget->EnableNavigationControllerEventListening();

  // Zooming and panning
  mitk::GlobalInteraction::GetInstance()->AddListener(
    multiWidget->GetMoveAndZoomInteractor()
  );

}

/**
\example Step8.cpp
*/
