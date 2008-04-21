#include "Step8.h"

#include "QmitkStdMultiWidget.h"

#include "mitkGlobalInteraction.h"
#include "mitkRenderingManager.h"

#include <qhbox.h>
#include <qvbox.h>

//##Documentation
//## @brief As Step6, but with QmitkStdMultiWidget as widget
Step8::Step8( int argc, char* argv[], QWidget *parent, const char *name )
  : Step6( argc, argv, parent, name )
{
}

void Step8::SetupWidgets()
{
  //Part I: Create windows and pass the tree to it
  // create toplevel widget with vertical layout
  m_TopLevelWidget = new QVBox(this);
  // create viewParent widget with horizontal layout
  QHBox* viewParent = new QHBox(m_TopLevelWidget);

  // Part Ia: create and initialize QmitkStdMultiWidget
  QmitkStdMultiWidget* multiWidget = new QmitkStdMultiWidget(viewParent);
  // create an iterator on the tree
  mitk::DataTreePreOrderIterator it(m_Tree);
  // tell the multiWidget which (part of) the tree to render
  multiWidget->SetData(&it);
  // initialize views as transversal, sagittal, coronar (from
  // top-left to bottom)
  mitk::RenderingManager::GetInstance()->InitializeViews(&it);
  // initialize bottom-right view as 3D view
  multiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
  // enable standard handler for levelwindow-slider
  multiWidget->EnableStandardLevelWindow();

  // add the displayed views to the tree to see their positions
  // in 2D and 3D
  multiWidget->AddDisplayPlaneSubTree(&it);

  //Part II: Setup standard interaction with the mouse
  // moving the cut-planes to click-point
  multiWidget->EnableNavigationControllerEventListening();
  // zooming and panning
  mitk::GlobalInteraction::GetInstance()->AddListener(
    multiWidget->GetMoveAndZoomInteractor()
  );

  setCentralWidget(m_TopLevelWidget);
}

/**
\example Step8.cpp
*/
