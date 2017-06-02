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

//! PROTOTYPE Qml item to integrate MITK render window into Qml application.
//!
//! See diagram qml-classes-mitk.jpg for an overall picture of involved classes.
//!
//! The structure implemented in these classes follows the framebuffer approach
//! described in
//! http://doc.qt.io/qt-5/qtquick-visualcanvas-scenegraph.html#mixing-scene-graph-and-opengl
//!
//! That is, MITK/VTK rendering is happening inside Qml's rendering thread and
//! in parallel to Qt's main thread. The rendering outputs to a GL framebuffer
//! that is used as a texture within the Qml scene.
//! Both Qt and VTK impose their structure on how rendering should be called,
//! see the two classes QVTKFramebufferObjectRenderer and vtkInternalOpenglRenderWindow.
//!
//! As in Qt widget based render windows, QmlMitkRenderWindowItem derives from
//! a Qt/VTK-integrating render window and adds its VtkPropRenderer to the VTK
//! scene. This VtkPropRenderer collects and organizes all MITK mappers and
//! makes them render when VTK asks to.
//!
//! Challenges for future development
//! * cleanup: example grew historically, structure could perhaps be simplified,
//!            responsibilities could be clearer.
//! * thread-safety: this example manages execution of MITK events within
//!            the Qt rendering thread. This allows access to VTK's GL context (e.g. for picking).
//!            An inherent risk are collisions on data structures around
//!            DataStorage: when rendering is threaded, the application
//!            could modify data storage elements that are currently being
//!            accessed by rendering. Working this out would probably
//!            require thread-safety on multiple levels: DataStorage hierarchy,
//!            DataNode methods, property (lists), and finally BaseData
//!            and all its derived classes.
//!
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

    QmlMitkRenderWindowItem(QQuickItem* parent = 0, const QString& name = "QML render window", mitk::VtkPropRenderer* renderer = nullptr, mitk::RenderingManager* renderingManager = nullptr);
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
    mitk::Point2D GetMousePositionFlipY(QMouseEvent* me) const;
    mitk::Point2D GetMousePosition(QWheelEvent* we) const;
    mitk::Point2D GetMousePositionFlipY(QWheelEvent* we) const;
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

protected:

    //! Add an event to the event queue to be executed in rendering.
    //!
    //! Avoids collisions between rendering and event handling,
    //! both of which might act on GL buffers (e.g. picking)
    void QueueEvent(mitk::InteractionEvent::Pointer e);

    //! List of events that should be treated during next rendering.
    std::vector<mitk::InteractionEvent::Pointer> m_PendingEvents;

    //! Executes (then clears) the list of unhandled UI events (m_PendingEvents).
    void processPendingEvents() override;
};

#endif
