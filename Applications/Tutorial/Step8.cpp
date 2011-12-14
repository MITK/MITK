/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "Step8.h"

#include "QmitkStdMultiWidget.h"

#include "mitkGlobalInteraction.h"
#include "mitkRenderingManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

//##Documentation
//## @brief As Step6, but with QmitkStdMultiWidget as widget
Step8::Step8(int argc, char* argv[], QWidget *parent) :
	Step6(argc, argv, parent)
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

	// Tell the multiWidget which DataStorage to render
	multiWidget->SetDataStorage(m_DataStorage);

	// Initialize views as transversal, sagittal, coronar (from
	// top-left to bottom)
	mitk::TimeSlicedGeometry::Pointer geo = m_DataStorage->ComputeBoundingGeometry3D(
			m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);

	// Initialize bottom-right view as 3D view
	multiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(
			mitk::BaseRenderer::Standard3D);

	// Enable standard handler for levelwindow-slider
	multiWidget->EnableStandardLevelWindow();

	// Add the displayed views to the DataStorage to see their positions in 2D and 3D
	multiWidget->AddDisplayPlaneSubTree();
	multiWidget->AddPlanesToDataStorage();
	multiWidget->SetWidgetPlanesVisibility(true);

	//*************************************************************************
	// Part II: Setup standard interaction with the mouse
	//*************************************************************************

	// Moving the cut-planes to click-point
	multiWidget->EnableNavigationControllerEventListening();
}
/**
 \example Step8.cpp
 */
