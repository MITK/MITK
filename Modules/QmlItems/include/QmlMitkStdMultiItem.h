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

#ifndef QmlMitkStdMultiItem_h
#define QmlMitkStdMultiItem_h

#include "MitkQmlItemsExports.h"

#include <mitkDataStorage.h>
#include <mitkMouseModeSwitcher.h>

#include <QObject>
#include <QQmlEngine>

class QmlMitkRenderWindowItem;

class MITKQMLITEMS_EXPORT QmlMitkStdMultiItem : public QObject
{
    Q_OBJECT

private:
    QmlMitkRenderWindowItem* m_viewerAxial;
    QmlMitkRenderWindowItem* m_viewerFrontal;
    QmlMitkRenderWindowItem* m_viewerSagittal;
    QmlMitkRenderWindowItem* m_viewerOriginal;

    mitk::DataNode::Pointer m_planeAxial;
    mitk::DataNode::Pointer m_planeFrontal;
    mitk::DataNode::Pointer m_planeSagittal;

    mitk::MouseModeSwitcher::Pointer m_mouseMode;

    static mitk::DataStorage::Pointer storage;

public:
    static QmlMitkStdMultiItem* instance;

    QmlMitkStdMultiItem();
    ~QmlMitkStdMultiItem();

    void registerViewerItem(QmlMitkRenderWindowItem* viewerItem);
    void moveCrossToPosition(const mitk::Point3D& newPosition);
    void addPlanes();
    const mitk::Point3D getCrossPosition() const;

    QmlMitkRenderWindowItem* getViewerAxial();
    QmlMitkRenderWindowItem* getViewerSagittal();
    QmlMitkRenderWindowItem* getViewerCoronal();
    QmlMitkRenderWindowItem* getViewerOriginal();

    static void create(QQmlEngine &engine, mitk::DataStorage::Pointer storage);
    public slots:
    void init();
    void togglePlanes();
};

#endif
