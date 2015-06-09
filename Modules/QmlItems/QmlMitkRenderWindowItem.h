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
#ifndef QmlMitkRenderWindowItem_h
#define QmlMitkRenderWindowItem_h

#include <QVTKQuickItem.h>

#include "mitkRenderWindowBase.h"

#include <QTimer>

#include "MitkQmlItemsExports.h"

/**
  \brief QML replacement for QmitkRenderWindow.

  A proof-of-concept render window. Currently exclusively
  for use in the QuickRender demo application.

  \warning Subject to change.
*/
class MITKQMLITEMS_EXPORT QmlMitkRenderWindowItem : public QVTKQuickItem, public mitk::RenderWindowBase
{
    Q_OBJECT

public:
    static QmlMitkRenderWindowItem* GetInstanceForVTKRenderWindow( vtkRenderWindow* rw );

    QmlMitkRenderWindowItem(QQuickItem* parent = 0,
                            const QString& name = "QML render window",
                            mitk::VtkPropRenderer* renderer = NULL,
                            mitk::RenderingManager* renderingManager = NULL);

    virtual vtkRenderWindow* GetVtkRenderWindow() override;
    virtual vtkRenderWindowInteractor* GetVtkRenderWindowInteractor() override;

    void SetDataStorage(mitk::DataStorage::Pointer storage);

    void InitView( mitk::BaseRenderer::MapperSlotId mapperID,
                   mitk::SliceNavigationController::ViewDirection viewDirection );

    void SetPlaneNodeParent( mitk::DataNode::Pointer node );

    void SetCrossHairPositioningOnClick(bool enabled);

signals:

public slots:

protected slots:

protected:

  virtual void init() override;
  virtual void prepareForRender() override;
  virtual void cleanupAfterRender() override;

  mitk::Point2D GetMousePosition(QMouseEvent* me) const;
  mitk::Point2D GetMousePosition(QWheelEvent* we) const;
  mitk::InteractionEvent::MouseButtons GetEventButton(QMouseEvent* me) const;
  mitk::InteractionEvent::MouseButtons GetButtonState(QMouseEvent* me) const;
  mitk::InteractionEvent::ModifierKeys GetModifiers(QInputEvent* me) const;
  mitk::InteractionEvent::MouseButtons GetButtonState(QWheelEvent* we) const;
  void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry) override; // !?

  virtual void mousePressEvent(QMouseEvent* e) override;
  virtual void mouseReleaseEvent(QMouseEvent* e) override;
  virtual void mouseMoveEvent(QMouseEvent* e) override;
  virtual void wheelEvent(QWheelEvent* e) override;

private slots:

private:
    mitk::DataStorage::Pointer m_DataStorage;
    mitk::DataNode::Pointer m_PlaneNodeParent;

    mitk::BaseRenderer::MapperSlotId m_MapperID;
    mitk::SliceNavigationController::ViewDirection m_ViewDirection;


    QTimer m_Animation;

    vtkSmartPointer<vtkEventQtSlotConnect> m_connect;

    static QMap<vtkRenderWindow*, QmlMitkRenderWindowItem*>& GetInstances();

};

#endif
