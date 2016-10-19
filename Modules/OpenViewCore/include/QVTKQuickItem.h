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

// original copyright below

/*========================================================================
 OpenView -- http://openview.kitware.com

 Copyright 2012 Kitware, Inc.

 Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/

#ifndef __QVTKQuickItem_h
#define __QVTKQuickItem_h

#include <QQuickFramebufferObject>

#include <QOpenGLShaderProgram>

#include "vtkSmartPointer.h"
#include "vtkNew.h"

#include <QMutex>

#include <MitkOpenViewCoreExports.h>

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QVTKMitkInteractorAdapter;
class QVTKInteractor;
class QVTKFramebufferObjectRenderer;
class vtkEventQtSlotConnect;
class vtkOpenGLRenderWindow;
class vtkObject;
class vtkContextView;

class MITKOPENVIEWCORE_EXPORT QVTKQuickItem : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    QVTKQuickItem(QQuickItem* parent = 0);

    // Description:
    // destructor
    ~QVTKQuickItem();

    Renderer *createRenderer() const;

    // Description:
    // get the render window used with this item
    vtkOpenGLRenderWindow* GetRenderWindow() const;

    // Description:
    // get the render window interactor used with this item
    // this item enforces its own interactor
    QVTKInteractor* GetInteractor() const;

    QMutex m_viewLock;

    protected slots:
    // slot called when vtk wants to know if the context is current
    virtual void IsCurrent(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);
    // slot called when vtk wants to know if a window is direct
    virtual void IsDirect(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);
    // slot called when vtk wants to know if a window supports OpenGL
    virtual void SupportsOpenGL(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);
    void onTextureFollowsItemSizeChanged(bool follows);

protected:
    // Called ONCE from the render thread before the FBO is first created and while the GUI thread is blocked
    virtual void init();
    // Called from the render thread BEFORE each update while the GUI thread blocked
    virtual bool prepareForRender();
    // Called from the render thread AFTER each update while the GUI thread is NOT blocked
    virtual void cleanupAfterRender();

    // handle item key events
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void keyReleaseEvent(QKeyEvent* e);

    // handle item mouse events
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void hoverEnterEvent(QHoverEvent* e);
    virtual void hoverLeaveEvent(QHoverEvent* e);
    virtual void hoverMoveEvent(QHoverEvent* e);
    QSGNode* updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData);

private:
    vtkOpenGLRenderWindow *m_win;
    vtkSmartPointer<QVTKInteractor> m_interactor;
    QVTKMitkInteractorAdapter* m_interactorAdapter;
    vtkSmartPointer<vtkEventQtSlotConnect> m_connect;

    friend class QVTKFramebufferObjectRenderer;
};

#endif
