/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#ifndef QVTKQuickItem_h
#define QVTKQuickItem_h

#include <QQuickItem>

#include <QOpenGLShaderProgram>

#include "vtkSmartPointer.h"
#include "vtkNew.h"

#include <QMutex>

#include "OpenViewCoreExports.h"

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QVTKInteractorAdapter;
class QVTKInteractor;
class vtkEventQtSlotConnect;
class vtkGenericOpenGLRenderWindow;
class vtkObject;
class vtkContextView;

class OVCORE_EXPORT QVTKQuickItem : public QQuickItem
{
  Q_OBJECT
public:
  QVTKQuickItem(QQuickItem* parent = 0);

  // Description:
  // destructor
  ~QVTKQuickItem();

  void itemChange(ItemChange change, const ItemChangeData &);

  // Description:
  // set the render window to use with this item
  void SetRenderWindow(vtkGenericOpenGLRenderWindow* win);

  // Description:
  // get the render window used with this item
  vtkGenericOpenGLRenderWindow* GetRenderWindow() const;

  // Description:
  // get the render window interactor used with this item
  // this item enforces its own interactor
  QVTKInteractor* GetInteractor() const;

public slots:
  virtual void paint();

protected slots:
  // slot to make this vtk render window current
  virtual void MakeCurrent();
  // slot called when vtk render window starts to draw
  virtual void Start();
  // slot called when vtk render window is done drawing
  virtual void End();
  // slot called when vtk wants to know if the context is current
  virtual void IsCurrent(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);
  // slot called when vtk wants to know if a window is direct
  virtual void IsDirect(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);
  // slot called when vtk wants to know if a window supports OpenGL
  virtual void SupportsOpenGL(vtkObject* caller, unsigned long vtk_event, void* client_data, void* call_data);

protected:

  virtual void init();
  virtual void prepareForRender();
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

  virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData);

  QMutex m_viewLock;

private:
  QOpenGLContext* m_context;
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_win;
  vtkSmartPointer<QVTKInteractor> m_interactor;
  QVTKInteractorAdapter* m_interactorAdapter;
  vtkSmartPointer<vtkEventQtSlotConnect> m_connect;

  bool m_InitCalledOnce;
};

#endif
