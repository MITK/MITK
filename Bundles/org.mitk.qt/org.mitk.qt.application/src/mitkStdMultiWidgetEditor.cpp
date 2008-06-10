#include "mitkStdMultiWidgetEditor.h"

#include <org.opencherry.ui/src/cherryUIException.h>

#include <QWidget>

#include <mitkGlobalInteraction.h>

#include "mitkDataStorageEditorInput.h"

const std::string mitkStdMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.stdmultiwidget";

mitkStdMultiWidgetEditor::mitkStdMultiWidgetEditor()
 : m_StdMultiWidget(0)
{
  
}

mitkStdMultiWidgetEditor::~mitkStdMultiWidgetEditor()
{
  if (m_StdMultiWidget != 0)
    delete m_StdMultiWidget;
}

void mitkStdMultiWidgetEditor::Init(cherry::IEditorSite::Pointer site, cherry::IEditorInput::Pointer input)
{
  if (input.Cast<mitkDataStorageEditorInput>().IsNull())
     throw cherry::PartInitException("Invalid Input: Must be IFileEditorInput");
  
  this->SetSite(site);
  this->SetInput(input);
}

void* mitkStdMultiWidgetEditor::CreatePartControl(void* parent)
{
  if (m_StdMultiWidget == 0)
  {
    QWidget* qtParent = static_cast<QWidget*>(parent);
    m_StdMultiWidget = new QmitkStdMultiWidget(qtParent);
    qtParent->layout()->addWidget(m_StdMultiWidget);
    
    mitk::DataTree::Pointer dataTree = this->GetEditorInput().Cast<mitkDataStorageEditorInput>()
      ->GetDataStorageReference()->GetDataTree();
    
    // Create an iterator on the tree
    mitk::DataTreePreOrderIterator it(dataTree);

    // Tell the multiWidget which (part of) the tree to render
    m_StdMultiWidget->SetData(&it);

    // Initialize views as transversal, sagittal, coronar (from
    // top-left to bottom)
    mitk::RenderingManager::GetInstance()->InitializeViews(&it);

    // Initialize bottom-right view as 3D view
    m_StdMultiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(
      mitk::BaseRenderer::Standard3D );

    // Enable standard handler for levelwindow-slider
    m_StdMultiWidget->EnableStandardLevelWindow();

    // Add the displayed views to the tree to see their positions
    // in 2D and 3D
    m_StdMultiWidget->AddDisplayPlaneSubTree(&it);
    
    m_StdMultiWidget->EnableNavigationControllerEventListening();
    
    mitk::GlobalInteraction::GetInstance()->AddListener(
        m_StdMultiWidget->GetMoveAndZoomInteractor()
      );
  }
  
  return m_StdMultiWidget;
}
  
void mitkStdMultiWidgetEditor::SetFocus()
{
  if (m_StdMultiWidget != 0)
    m_StdMultiWidget->setFocus();
}
