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

#include "QmitkNavigationButtonsView.h"

#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include "mitkResliceMethodProperty.h"
#include "mitkRenderingManager.h"

#include "mitkDiffusionImage.h"
#include "mitkPlanarFigure.h"
#include "mitkFiberBundle.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "mitkFiberBundleInteractor.h"
#include "mitkPlanarFigureInteractor.h"

#include "mitkGlobalInteraction.h"

#include "mitkGeometry2D.h"

#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"
#include "berryISelectionService.h"
#include "berryConstants.h"
#include "berryPlatformUI.h"

#include "itkRGBAPixel.h"
#include "itkTractsToProbabilityImageFilter.h"

#include "qwidgetaction.h"
#include "qcolordialog.h"

const std::string QmitkNavigationButtonsView::VIEW_ID = "org.mitk.views.NavigationButtonsview";

using namespace berry;


QmitkNavigationButtonsView::QmitkNavigationButtonsView()
  : QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL),
{
}

QmitkNavigationButtonsView::QmitkNavigationButtonsView(const QmitkNavigationButtonsView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkNavigationButtonsView::~QmitkNavigationButtonsView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}



void QmitkNavigationButtonsView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationButtonsViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

  }
}


void QmitkNavigationButtonsView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkNavigationButtonsView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkNavigationButtonsView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TextureIntON), SIGNAL(clicked()), this, SLOT(TextIntON()) );
  }
}

void QmitkNavigationButtonsView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkNavigationButtonsView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

int QmitkNavigationButtonsView::GetSizeFlags(bool width)
{
  if(!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkNavigationButtonsView::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
{
  if(width==false)
  {
    return m_FoundSingleOdfImage ? 120 : 80;
  }
  else
  {
    return preferredResult;
  }
}
