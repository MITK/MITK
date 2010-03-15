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

#include "QmitkSliceWidget.h"
#include "QmitkStepperAdapter.h"
#include "mitkNodePredicateDataType.h"
//#include "QmitkRenderWindow.h"
//
//#include "mitkSliceNavigationController.h"
//#include "QmitkLevelWindowWidget.h"
//
//#include <vtkRenderer.h>
//#include "mitkRenderingManager.h"

#include <QMenu>
#include <QMouseEvent>

QmitkSliceWidget::QmitkSliceWidget(QWidget* parent, const char* name,
		Qt::WindowFlags f) :
	QWidget(parent, f)
{
	this->setupUi(this);

	if (name != 0)
		this->setObjectName(name);

	popUp = new QMenu(this);
	popUp->addAction("Transversal");
	popUp->addAction("Frontal");
	popUp->addAction("Sagittal");

	QObject::connect(popUp, SIGNAL(triggered(QAction*)), this, SLOT(ChangeView(QAction*)) );
	setPopUpEnabled(false);

	m_SlicedGeometry = 0;
	m_View = mitk::SliceNavigationController::Transversal;

	QHBoxLayout *hlayout = new QHBoxLayout(container);
	hlayout->setMargin(0);

	// create widget
	QString composedName("QmitkSliceWidget::");
	if (!this->objectName().isEmpty())
		composedName += this->objectName();
	else
		composedName += "QmitkGLWidget";
	m_RenderWindow = new QmitkRenderWindow(container, composedName);
	m_Renderer = m_RenderWindow->GetRenderer();
	hlayout->addWidget(m_RenderWindow);

	new QmitkStepperAdapter(m_NavigatorWidget,
			m_RenderWindow->GetSliceNavigationController()->GetSlice(),
			"navigation");

	SetLevelWindowEnabled(true);

}

mitk::VtkPropRenderer* QmitkSliceWidget::GetRenderer()
{
	return m_Renderer;
}

QFrame* QmitkSliceWidget::GetSelectionFrame()
{
	return SelectionFrame;
}

void QmitkSliceWidget::SetDataStorage(
		mitk::StandaloneDataStorage::Pointer storage)
{
	m_DataStorage = storage;
	m_Renderer->SetDataStorage(m_DataStorage);
}

mitk::StandaloneDataStorage* QmitkSliceWidget::GetDataStorage()
{
	if (m_DataStorage.IsNotNull())
	{
		return m_DataStorage;
	}
	else
	{
		return NULL;
	}
}

void QmitkSliceWidget::SetData(
		mitk::DataStorage::SetOfObjects::ConstIterator it)
{
	SetData(it->Value(), m_View);
}

void QmitkSliceWidget::SetData(
		mitk::DataStorage::SetOfObjects::ConstIterator it,
		mitk::SliceNavigationController::ViewDirection view)
{
	SetData(it->Value(), view);
}

void QmitkSliceWidget::SetData(mitk::DataNode::Pointer node)
{
	try
	{
		if (m_DataStorage.IsNotNull())
		{
			m_DataStorage->Add(node);
		}
	} catch (...)
	{
	}
	SetData(node, m_View);
}

//void QmitkSliceWidget::AddData( mitk::DataNode::Pointer node)
//{
//  if ( m_DataTree.IsNull() )
//  {
//    m_DataTree = mitk::DataTree::New();
//  }
//  mitk::DataTreePreOrderIterator it(m_DataTree);
//  it.Add( node );
//  SetData(&it, m_View);
//}


void QmitkSliceWidget::SetData(mitk::DataNode::Pointer /*treeNode*/,
		mitk::SliceNavigationController::ViewDirection view)
{
	try
	{
		if (m_DataStorage.IsNotNull())
		{
			levelWindow->SetDataStorage(m_DataStorage);
			mitk::DataStorage::SetOfObjects::ConstPointer rs =
					m_DataStorage->GetSubset(mitk::NodePredicateDataType::New(
							"Image"));
			mitk::DataStorage::SetOfObjects::ConstIterator it;
			bool noVisibleImage = true;
			for (it = rs->Begin(); it != rs->End(); ++it)
			{
				mitk::DataNode::Pointer node = it.Value();
				node->SetName("currentImage");
				mitk::Image::Pointer image = m_DataStorage->GetNamedObject<
						mitk::Image> ("currentImage");

				if (image.IsNotNull() && node->IsVisible(GetRenderer()))
				{
					m_SlicedGeometry = image->GetSlicedGeometry();
					mitk::LevelWindow picLevelWindow;
					node->GetLevelWindow(picLevelWindow, NULL);
					noVisibleImage = false;
					break;
				}
			}

			if (noVisibleImage)
				MITK_INFO << " No image visible!";

			GetRenderer()->SetDataStorage(m_DataStorage);
		}
		InitWidget(view);
	} catch (...)
	{
	}
}

void QmitkSliceWidget::InitWidget(
		mitk::SliceNavigationController::ViewDirection viewDirection)
{
	m_View = viewDirection;

	mitk::SliceNavigationController* controller =
			m_RenderWindow->GetSliceNavigationController();

	if (viewDirection == mitk::SliceNavigationController::Transversal)
	{
		controller->SetViewDirection(
				mitk::SliceNavigationController::Transversal);
	}
	else if (viewDirection == mitk::SliceNavigationController::Frontal)
	{
		controller->SetViewDirection(mitk::SliceNavigationController::Frontal);
	}
	// init sagittal view
	else
	{
		controller->SetViewDirection(mitk::SliceNavigationController::Sagittal);
	}

	int currentPos = 0;
	if (m_RenderWindow->GetSliceNavigationController())
	{
		currentPos = controller->GetSlice()->GetPos();
	}

	if (m_SlicedGeometry.IsNull())
	{
		return;
	}

	// compute bounding box with respect to first images geometry
	const mitk::BoundingBox::BoundsArrayType imageBounds =
			m_SlicedGeometry->GetBoundingBox()->GetBounds();

	//  mitk::SlicedGeometry3D::Pointer correctGeometry = m_SlicedGeometry.GetPointer();
	mitk::Geometry3D::Pointer
			geometry =
					static_cast<mitk::Geometry3D*> (m_SlicedGeometry->Clone().GetPointer());

	const mitk::BoundingBox::Pointer boundingbox =
			m_DataStorage->ComputeVisibleBoundingBox(GetRenderer(), NULL);
	if (boundingbox->GetPoints()->Size() > 0)
	{
		////geometry = mitk::Geometry3D::New();
		////geometry->Initialize();
		//geometry->SetBounds(boundingbox->GetBounds());
		//geometry->SetSpacing(correctGeometry->GetSpacing());

		//let's see if we have data with a limited live-span ...
		mitk::TimeBounds timebounds = m_DataStorage->ComputeTimeBounds(
				GetRenderer(), NULL);

		if (timebounds[1] < mitk::ScalarTypeNumericTraits::max())
		{
			mitk::ScalarType duration = timebounds[1] - timebounds[0];

			mitk::TimeSlicedGeometry::Pointer timegeometry =
					mitk::TimeSlicedGeometry::New();

			timegeometry->InitializeEvenlyTimed(geometry.GetPointer(),
					(unsigned int) duration);

			timegeometry->SetTimeBounds(timebounds); //@bug really required? FIXME

			timebounds[1] = timebounds[0] + 1.0f;
			geometry->SetTimeBounds(timebounds);

			geometry = timegeometry;
		}

		if (const_cast<mitk::BoundingBox*> (geometry->GetBoundingBox())->GetDiagonalLength2()
				>= mitk::eps)
		{
			controller->SetInputWorldGeometry(geometry);
			controller->Update();
		}
	}

	GetRenderer()->GetDisplayGeometry()->Fit();
	mitk::RenderingManager::GetInstance()->RequestUpdate(
			GetRenderer()->GetRenderWindow());
	//int w=vtkObject::GetGlobalWarningDisplay();
	//vtkObject::GlobalWarningDisplayOff();
	//vtkRenderer * vtkrenderer = ((mitk::OpenGLRenderer*)(GetRenderer()))->GetVtkRenderer();
	//if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
	//vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkSliceWidget::UpdateGL()
{
	GetRenderer()->GetDisplayGeometry()->Fit();
	mitk::RenderingManager::GetInstance()->RequestUpdate(
			GetRenderer()->GetRenderWindow());
}

void QmitkSliceWidget::mousePressEvent(QMouseEvent * e)
{
	if (e->button() == Qt::RightButton && popUpEnabled)
	{
		popUp->popup(QCursor::pos());
	}
}

void QmitkSliceWidget::wheelEvent(QWheelEvent * e)
{
	int val = m_NavigatorWidget->GetPos();

	if (e->orientation() * e->delta() > 0)
	{
		m_NavigatorWidget->SetPos(val + 1);
	}
	else
	{
		if (val > 0)
			m_NavigatorWidget->SetPos(val - 1);
	}
}

void QmitkSliceWidget::ChangeView(QAction* val)
{
	if (val->text() == "Transversal")
	{
		InitWidget(mitk::SliceNavigationController::Transversal);
	}
	else if (val->text() == "Frontal")
	{
		InitWidget(mitk::SliceNavigationController::Frontal);
	}
	else if (val->text() == "Sagittal")
	{
		InitWidget(mitk::SliceNavigationController::Sagittal);
	}
}

void QmitkSliceWidget::setPopUpEnabled(bool b)
{
	popUpEnabled = b;
}

QmitkSliderNavigatorWidget* QmitkSliceWidget::GetNavigatorWidget()
{
	return m_NavigatorWidget;
}

void QmitkSliceWidget::SetLevelWindowEnabled(bool enable)
{
	levelWindow->setEnabled(enable);
	if (!enable)
	{
		levelWindow->setMinimumWidth(0);
		levelWindow->setMaximumWidth(0);
	}
	else
	{
		levelWindow->setMinimumWidth(28);
		levelWindow->setMaximumWidth(28);
	}
}

bool QmitkSliceWidget::IsLevelWindowEnabled()
{
	return levelWindow->isEnabled();
}

QmitkRenderWindow* QmitkSliceWidget::GetRenderWindow()
{
	return m_RenderWindow;
}

mitk::SliceNavigationController*
QmitkSliceWidget::GetSliceNavigationController() const
{
	return m_RenderWindow->GetSliceNavigationController();
}

mitk::CameraRotationController*
QmitkSliceWidget::GetCameraRotationController() const
{
	return m_RenderWindow->GetCameraRotationController();
}

mitk::BaseController*
QmitkSliceWidget::GetController() const
{
	return m_RenderWindow->GetController();
}
