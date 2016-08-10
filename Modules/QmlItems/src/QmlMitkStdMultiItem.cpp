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

#include "QmlMitkStdMultiItem.h"
#include "QmlMitkRenderWindowItem.h"

#include <mitkImage.h>
#include <mitkDataNode.h>
#include <mitkIOUtil.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkLine.h>

QmlMitkStdMultiItem* QmlMitkStdMultiItem::instance = nullptr;
mitk::DataStorage::Pointer QmlMitkStdMultiItem::storage = nullptr;

QmlMitkStdMultiItem::QmlMitkStdMultiItem()
{
    instance = this;
}

QmlMitkStdMultiItem::~QmlMitkStdMultiItem()
{
}

void QmlMitkStdMultiItem::registerViewerItem(QmlMitkRenderWindowItem *viewerItem)
{
    viewerItem->setupView();
    viewerItem->createPlaneNode();

    switch (viewerItem->GetRenderer()->GetSliceNavigationController()->GetDefaultViewDirection())
    {
        case mitk::SliceNavigationController::Axial:
            this->m_viewerAxial = viewerItem;
            break;
        case mitk::SliceNavigationController::Frontal:
            this->m_viewerFrontal = viewerItem;
            break;
        case mitk::SliceNavigationController::Sagittal:
            this->m_viewerSagittal = viewerItem;
            break;
        default:
            this->m_viewerOriginal = viewerItem;
    }
}

void QmlMitkStdMultiItem::init()
{
    if(QmlMitkStdMultiItem::storage.IsNull())
        QmlMitkStdMultiItem::storage = mitk::StandaloneDataStorage::New();

    this->m_mouseMode = mitk::MouseModeSwitcher::New("QmlMitkStdMultiItem");
    this->m_mouseMode->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::MITK);

    this->m_viewerAxial->SetDataStorage(QmlMitkStdMultiItem::storage);
    this->m_viewerFrontal->SetDataStorage(QmlMitkStdMultiItem::storage);
    this->m_viewerSagittal->SetDataStorage(QmlMitkStdMultiItem::storage);
    this->m_viewerOriginal->SetDataStorage(QmlMitkStdMultiItem::storage);

    this->addPlanes();

    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(QmlMitkStdMultiItem::storage);
}

void QmlMitkStdMultiItem::togglePlanes()
{
    bool toggle;
    this->m_planeAxial->GetBoolProperty("visible", toggle);

    this->m_planeAxial->SetVisibility(!toggle);
    this->m_planeFrontal->SetVisibility(!toggle);
    this->m_planeSagittal->SetVisibility(!toggle);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmlMitkStdMultiItem::addPlanes()
{
    this->m_planeAxial      = this->m_viewerAxial->GetRenderer()->GetCurrentWorldPlaneGeometryNode();
    this->m_planeFrontal    = this->m_viewerFrontal->GetRenderer()->GetCurrentWorldPlaneGeometryNode();
    this->m_planeSagittal   = this->m_viewerSagittal->GetRenderer()->GetCurrentWorldPlaneGeometryNode();

    QmlMitkStdMultiItem::storage->Add(this->m_planeAxial);
    QmlMitkStdMultiItem::storage->Add(this->m_planeFrontal);
    QmlMitkStdMultiItem::storage->Add(this->m_planeSagittal);
}

void QmlMitkStdMultiItem::moveCrossToPosition(const mitk::Point3D& newPosition)
{
    this->m_viewerAxial->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
    this->m_viewerFrontal->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
    this->m_viewerSagittal->GetSliceNavigationController()->SelectSliceByPoint(newPosition);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

const mitk::Point3D QmlMitkStdMultiItem::getCrossPosition() const
{
    const mitk::PlaneGeometry *plane1 = this->m_viewerAxial->GetSliceNavigationController()->GetCurrentPlaneGeometry();
    const mitk::PlaneGeometry *plane2 = this->m_viewerFrontal->GetSliceNavigationController()->GetCurrentPlaneGeometry();
    const mitk::PlaneGeometry *plane3 = this->m_viewerSagittal->GetSliceNavigationController()->GetCurrentPlaneGeometry();

    mitk::Line3D line;
    if ( (plane1 != NULL) && (plane2 != NULL)
        && (plane1->IntersectionLine( plane2, line )) )
    {
        mitk::Point3D point;
        if ( (plane3 != NULL)
            && (plane3->IntersectionPoint( line, point )) )
        {
            return point;
        }
    }
    // TODO BUG POSITIONTRACKER;
    mitk::Point3D p;
    return p;
    //return m_LastLeftClickPositionSupplier->GetCurrentPoint();
}


QmlMitkRenderWindowItem* QmlMitkStdMultiItem::getViewerAxial()
{
    return this->m_viewerAxial;
}

QmlMitkRenderWindowItem* QmlMitkStdMultiItem::getViewerCoronal()
{
    return this->m_viewerFrontal;
}

QmlMitkRenderWindowItem* QmlMitkStdMultiItem::getViewerSagittal()
{
    return this->m_viewerSagittal;
}

QmlMitkRenderWindowItem* QmlMitkStdMultiItem::getViewerOriginal()
{
    return this->m_viewerOriginal;
}

void QmlMitkStdMultiItem::create(QQmlEngine &engine, mitk::DataStorage::Pointer storage)
{
    QmlMitkStdMultiItem::storage = storage;

    qmlRegisterType<QmlMitkStdMultiItem>("Mitk.Views", 1, 0, "MultiItem");
    qmlRegisterType<QmlMitkRenderWindowItem>("Mitk.Views", 1, 0, "ViewItem");
    QQmlComponent component(&engine, QUrl("qrc:/views/MitkStdMultiItem.qml"));
}
