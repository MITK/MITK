/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkRegionGrowingView.h"
#include "QmitkStdMultiWidget.h"

const std::string QmitkRegionGrowingView::VIEW_ID = "org.mitk.views.regiongrowing";

QmitkRegionGrowingView::QmitkRegionGrowingView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
}

QmitkRegionGrowingView::~QmitkRegionGrowingView()
{
}

void QmitkRegionGrowingView::Deactivated()
{
  m_Controls->m_AdaptiveRGWidget->Deactivated();
}

void QmitkRegionGrowingView::Activated()
{
  m_Controls->m_AdaptiveRGWidget->Activated();
}

void QmitkRegionGrowingView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkRegionGrowingViewControls;
    m_Controls->setupUi( parent );

    m_Controls->m_AdaptiveRGWidget->SetDataStorage(this->GetDataStorage());
  }
}

void QmitkRegionGrowingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
  m_Controls->m_AdaptiveRGWidget->SetMultiWidget(&stdMultiWidget);
}


void QmitkRegionGrowingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void QmitkRegionGrowingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  // iterate all selected objects, adjust warning visibility
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
       it != nodes.end();
       ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls->lblWarning->setVisible( false );
      m_Controls->m_AdaptiveRGWidget->EnableControls(true);
      m_Controls->m_AdaptiveRGWidget->SetInputImageNode(node);
      return;
    }
  }

  m_Controls->lblWarning->setVisible( true );
  m_Controls->m_AdaptiveRGWidget->EnableControls(false);
}
