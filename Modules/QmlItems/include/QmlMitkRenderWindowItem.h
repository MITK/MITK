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

#ifndef __QmlMitkRenderWindowItem_h
#define __QmlMitkRenderWindowItem_h

#include "MitkQmlItemsExports.h"

#include "QVTKQuickItem.h"

#include "mitkRenderWindowBase.h"

#include <vtkCornerAnnotation.h>
#include <vtkMitkRectangleProp.h>

class QmlMitkStdMultiItem;

class MITKQMLITEMS_EXPORT QmlMitkRenderWindowItem : public QVTKQuickItem, public mitk::RenderWindowBase
{
    Q_OBJECT
    Q_PROPERTY(int viewType READ getViewType WRITE setViewType NOTIFY viewTypeChanged);
    Q_PROPERTY(QmlMitkStdMultiItem* multiItem READ getMultiItem WRITE setMultiItem NOTIFY multiItemChanged);
private:
    QmlMitkStdMultiItem*                    m_multiItem;
    vtkSmartPointer<vtkCornerAnnotation>    m_annotation;
    vtkSmartPointer<vtkMitkRectangleProp>   m_rectangle;

    int m_viewType;

public:
    static QmlMitkRenderWindowItem* instance;
    static QmlMitkRenderWindowItem* GetInstanceForVTKRenderWindow( vtkRenderWindow* rw );

    QmlMitkRenderWindowItem(QQuickItem* parent = 0, const QString& name = "QML render window", mitk::VtkPropRenderer* renderer = NULL, mitk::RenderingManager* renderingManager = NULL);
    ~QmlMitkRenderWindowItem();

    virtual vtkRenderWindow* GetVtkRenderWindow();
    virtual vtkRenderWindowInteractor* GetVtkRenderWindowInteractor();

    void SetDataStorage(mitk::DataStorage::Pointer storage);
    void InitView( mitk::BaseRenderer::MapperSlotId mapperID, mitk::SliceNavigationController::ViewDirection viewDirection );

    virtual bool prepareForRender();
    virtual void cleanupAfterRender();
    void createPlaneNode();

    void setMultiItem(QmlMitkStdMultiItem* multiItem);
    QmlMitkStdMultiItem* getMultiItem();

    void setDecorationProperties(std::string text, mitk::Color color);

    void setViewType(int type);
    int getViewType();

    void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry);

    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);

    mitk::Point2D GetMousePosition(QMouseEvent* me) const;
    mitk::Point2D GetMousePosition(QWheelEvent* we) const;
    mitk::InteractionEvent::MouseButtons GetEventButton(QMouseEvent* me) const;
    mitk::InteractionEvent::MouseButtons GetButtonState(QMouseEvent* me) const;
    mitk::InteractionEvent::ModifierKeys GetModifiers(QInputEvent* me) const;
    mitk::InteractionEvent::MouseButtons GetButtonState(QWheelEvent* we) const;

    static QMap<vtkRenderWindow*, QmlMitkRenderWindowItem*>& GetInstances();

public slots:
    void setupView();

signals:
    void multiItemChanged();
    void viewTypeChanged();
};

#endif
