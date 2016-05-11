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

#include "QmlMitkRenderWindowItem.h"

#include <QVTKInteractor.h>
#include <QVTKInteractorAdapter.h>

#include <vtkEventQtSlotConnect.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkTextProperty.h>

#include "mitkMouseWheelEvent.h"
#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseDoubleClickEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkInternalEvent.h"
#include "mitkPlaneGeometryDataMapper2D.h"
#include "mitkCameraController.h"

#include "QmlMitkStdMultiItem.h"
#include <QQuickWindow>

QmlMitkRenderWindowItem* QmlMitkRenderWindowItem::instance = nullptr;

QmlMitkRenderWindowItem* QmlMitkRenderWindowItem::GetInstanceForVTKRenderWindow(vtkRenderWindow* rw)
{
    if (GetInstances().contains(rw))
    {
        return GetInstances()[rw];
    }
    return 0;
}

QMap<vtkRenderWindow*, QmlMitkRenderWindowItem*>& QmlMitkRenderWindowItem::GetInstances()
{
    static QMap<vtkRenderWindow*, QmlMitkRenderWindowItem*> s_Instances;
    return s_Instances;
}

QmlMitkRenderWindowItem::QmlMitkRenderWindowItem(QQuickItem* parent, const QString& name, mitk::VtkPropRenderer* , mitk::RenderingManager* renderingManager) : QVTKQuickItem(parent)
{
    instance = this;

    mitk::RenderWindowBase::Initialize(renderingManager, name.toStdString().c_str());
    GetInstances()[QVTKQuickItem::GetRenderWindow()] = this;

    this->m_annotation = vtkSmartPointer<vtkCornerAnnotation>::New();
    this->m_rectangle  = vtkSmartPointer<vtkMitkRectangleProp>::New();
}

void QmlMitkRenderWindowItem::createPlaneNode()
{
    mitk::DataStorage::Pointer m_DataStorage = mitk::RenderWindowBase::GetRenderer()->GetDataStorage();
    if (m_DataStorage.IsNotNull())
    {
        mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
    }

    if (this->GetRenderer()->GetSliceNavigationController()->GetDefaultViewDirection() == mitk::SliceNavigationController::Original)
        return;

    mitk::DataNode::Pointer   planeNode;
    mitk::IntProperty::Pointer  layer;
    mitk::PlaneGeometryDataMapper2D::Pointer mapper = mitk::PlaneGeometryDataMapper2D::New();

    layer = mitk::IntProperty::New(1000);

    planeNode = this->GetRenderer()->GetCurrentWorldPlaneGeometryNode();

    planeNode->SetProperty("visible", mitk::BoolProperty::New(true));
    planeNode->SetProperty("name", mitk::StringProperty::New("plane"));
    planeNode->SetProperty("isPlane", mitk::BoolProperty::New(true));
    planeNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
    planeNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    planeNode->SetProperty("layer", layer);
    planeNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

    switch (this->GetRenderer()->GetSliceNavigationController()->GetDefaultViewDirection())
    {
        case mitk::SliceNavigationController::Axial:
            planeNode->SetColor(0.88, 0.35, 0.27);
            break;
        case mitk::SliceNavigationController::Sagittal:
            planeNode->SetColor(0.25, 0.7, 0.35);
            break;
        case mitk::SliceNavigationController::Frontal:
            planeNode->SetColor(0.01, 0.31, 0.67);
            break;
        default:
            planeNode->SetColor(1.0, 1.0, 0.0);
    }

}

void QmlMitkRenderWindowItem::setViewType(int viewType)
{
    this->m_viewType = viewType;

    emit this->viewTypeChanged();
}

void QmlMitkRenderWindowItem::setDecorationProperties(std::string text, mitk::Color color)
{
    this->m_annotation->SetText(0, text.c_str());
    this->m_annotation->SetMaximumFontSize(12);
    this->m_annotation->GetTextProperty()->SetColor( color[0],color[1],color[2] );


    if(!this->GetRenderer()->GetVtkRenderer()->HasViewProp(this->m_annotation))
    {
        this->GetRenderer()->GetVtkRenderer()->AddViewProp(this->m_annotation);
    }

    this->m_rectangle->SetColor(color[0],color[1],color[2]);

    if(!this->GetRenderer()->GetVtkRenderer()->HasViewProp(this->m_rectangle))
    {
        this->GetRenderer()->GetVtkRenderer()->AddViewProp(this->m_rectangle);
    }
}

void QmlMitkRenderWindowItem::setupView()
{
    switch (this->m_viewType)
    {
        case 0:
            this->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
            this->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
            break;
        case 1:
            this->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
            this->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Frontal);
            break;
        case 2:
            this->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
            this->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
            break;
        case 3:
            this->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
            this->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Original);
        break;
        default:
            break;
    }
}

void QmlMitkRenderWindowItem::setMultiItem(QmlMitkStdMultiItem* multiItem)
{
    if(this->m_multiItem == multiItem)
        return;

    this->m_multiItem = multiItem;
    this->m_multiItem->registerViewerItem(this);
}

QmlMitkStdMultiItem* QmlMitkRenderWindowItem::getMultiItem()
{
    return this->m_multiItem;
}

void QmlMitkRenderWindowItem::SetDataStorage(mitk::DataStorage::Pointer storage)
{
    this->GetRenderer()->SetDataStorage(storage);
}

mitk::Point2D QmlMitkRenderWindowItem::GetMousePosition(QMouseEvent* me) const
{
    qreal ratio = this->window()->effectiveDevicePixelRatio();
    mitk::Point2D point;
    point[0] = me->x()*ratio;
    point[1] = me->y()*ratio;
    return point;
}

mitk::Point2D QmlMitkRenderWindowItem::GetMousePosition(QWheelEvent* we) const
{
    qreal ratio = this->window()->effectiveDevicePixelRatio();
    mitk::Point2D point;
    point[0] = we->x()*ratio;
    point[1] = we->y()*ratio;
    return point;
}

mitk::InteractionEvent::MouseButtons QmlMitkRenderWindowItem::GetEventButton(QMouseEvent* me) const
{
    mitk::InteractionEvent::MouseButtons eventButton;
    switch (me->button())
    {
        case Qt::LeftButton:
            eventButton = mitk::InteractionEvent::LeftMouseButton;
            break;
        case Qt::RightButton:
            eventButton = mitk::InteractionEvent::RightMouseButton;
            break;
        case Qt::MidButton:
            eventButton = mitk::InteractionEvent::MiddleMouseButton;
            break;
        default:
            eventButton = mitk::InteractionEvent::NoButton;
            break;
    }
    return eventButton;
}

mitk::InteractionEvent::MouseButtons QmlMitkRenderWindowItem::GetButtonState(QMouseEvent* me) const
{
    mitk::InteractionEvent::MouseButtons buttonState = mitk::InteractionEvent::NoButton;

    if (me->buttons() & Qt::LeftButton)
    {
        buttonState = buttonState | mitk::InteractionEvent::LeftMouseButton;
    }
    if (me->buttons() & Qt::RightButton)
    {
        buttonState = buttonState | mitk::InteractionEvent::RightMouseButton;
    }
    if (me->buttons() & Qt::MidButton)
    {
        buttonState = buttonState | mitk::InteractionEvent::MiddleMouseButton;
    }
    return buttonState;
}

mitk::InteractionEvent::ModifierKeys QmlMitkRenderWindowItem::GetModifiers(QInputEvent* me) const
{
    mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::NoKey;

    if (me->modifiers() & Qt::ALT)
    {
        modifiers = modifiers | mitk::InteractionEvent::AltKey;
    }
    if (me->modifiers() & Qt::CTRL)
    {
        modifiers = modifiers | mitk::InteractionEvent::ControlKey;
    }
    if (me->modifiers() & Qt::SHIFT)
    {
        modifiers = modifiers | mitk::InteractionEvent::ShiftKey;
    }
    return modifiers;
}

mitk::InteractionEvent::MouseButtons QmlMitkRenderWindowItem::GetButtonState(QWheelEvent* we) const
{
    mitk::InteractionEvent::MouseButtons buttonState = mitk::InteractionEvent::NoButton;

    if (we->buttons() & Qt::LeftButton)
    {
        buttonState = buttonState | mitk::InteractionEvent::LeftMouseButton;
    }
    if (we->buttons() & Qt::RightButton)
    {
        buttonState = buttonState | mitk::InteractionEvent::RightMouseButton;
    }
    if (we->buttons() & Qt::MidButton)
    {
        buttonState = buttonState | mitk::InteractionEvent::MiddleMouseButton;
    }
    return buttonState;
}


int QmlMitkRenderWindowItem::getViewType()
{
    return this->m_viewType;
}

void QmlMitkRenderWindowItem::mousePressEvent(QMouseEvent* me)
{
    mitk::Point2D mousePosition = GetMousePosition(me);
    //mousePosition[1] = this->GetRenderer()->GetSizeY() - mousePosition[1];

    mitk::MousePressEvent::Pointer mPressEvent =
    mitk::MousePressEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition, GetButtonState(me), GetModifiers(me), GetEventButton(me));

    mitk::RenderWindowBase::HandleEvent(mPressEvent.GetPointer());
    QVTKQuickItem::mousePressEvent(me);
}

void QmlMitkRenderWindowItem::mouseReleaseEvent(QMouseEvent* me)
{
    mitk::Point2D mousePosition = GetMousePosition(me);
    //mousePosition[1] = this->GetRenderer()->GetSizeY() - mousePosition[1];

    mitk::MouseReleaseEvent::Pointer mReleaseEvent =
    mitk::MouseReleaseEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition, GetButtonState(me), GetModifiers(me), GetEventButton(me));

    mitk::RenderWindowBase::HandleEvent(mReleaseEvent.GetPointer());
    QVTKQuickItem::mouseReleaseEvent(me);
}

void QmlMitkRenderWindowItem::mouseMoveEvent(QMouseEvent* me)
{
    mitk::Point2D mousePosition = GetMousePosition(me);
    //mousePosition[1] = this->GetRenderer()->GetSizeY() - mousePosition[1];

    mitk::MouseMoveEvent::Pointer mMoveEvent =
    mitk::MouseMoveEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition, GetButtonState(me), GetModifiers(me));

    mitk::RenderWindowBase::HandleEvent(mMoveEvent.GetPointer());
    QVTKQuickItem::mouseMoveEvent(me);
}

void QmlMitkRenderWindowItem::wheelEvent(QWheelEvent *we)
{
    mitk::Point2D mousePosition = GetMousePosition(we);

    mitk::MouseWheelEvent::Pointer mWheelEvent =
    mitk::MouseWheelEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition, GetButtonState(we), GetModifiers(we), we->delta());

    mitk::RenderWindowBase::HandleEvent(mWheelEvent.GetPointer());
    QVTKQuickItem::wheelEvent(we);
}

bool QmlMitkRenderWindowItem::prepareForRender()
{

    mitk::VtkPropRenderer *vPR = dynamic_cast<mitk::VtkPropRenderer*>(mitk::BaseRenderer::GetInstance(this->GetRenderWindow()));
    if (vPR)
    {
        vPR->PrepareRender();
    }

    mitk::RenderWindowBase::GetRenderer()->ForceImmediateUpdate();

    return true;
}

void QmlMitkRenderWindowItem::cleanupAfterRender()
{
}

void QmlMitkRenderWindowItem::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
    QVTKQuickItem::geometryChanged(newGeometry, oldGeometry);
    mitk::BaseRenderer::GetInstance(this->GetRenderWindow())->GetCameraController()->Fit();
}

QmlMitkRenderWindowItem::~QmlMitkRenderWindowItem()
{
    this->Destroy();
}

vtkRenderWindow* QmlMitkRenderWindowItem::GetVtkRenderWindow()
{
    return QVTKQuickItem::GetRenderWindow();
}

vtkRenderWindowInteractor* QmlMitkRenderWindowItem::GetVtkRenderWindowInteractor()
{
    return QVTKQuickItem::GetInteractor();
}
