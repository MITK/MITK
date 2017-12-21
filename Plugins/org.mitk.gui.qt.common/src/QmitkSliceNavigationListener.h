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


#ifndef __Q_MITK_SLICE_NAVIGATION_LISTENER_H
#define __Q_MITK_SLICE_NAVIGATION_LISTENER_H

#include <itkEventObject.h>

#include <QObject>

#include <org_mitk_gui_qt_common_Export.h>

namespace itk
{
  class Object;
}

namespace mitk
{
  class SliceNavigationController;
  struct IRenderWindowPart;
}


/** @brief Helper class to allow QmitkAbstractView and derived classes to react on changes of the slice/time navigation.
 Purpose of the class to be used in view and to give the respective view class (by composition) the posibility to react
 on changes of the currently selected timepoint or position in the world geometry.\n
 In order to setup this class properly the following things must be regarded:
 - View class must also derive public from mitk::IRenderWindowPartListener
 - View class must implement void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) and pass the renderWindowPart to the listener.

    void QmitkMyView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
    {
      this->m_SliceNavigationListener.RenderWindowPartActivated(renderWindowPart);
    }

 - View class must implement void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) and pass the renderWindowPart to the listener.

    void QmitkMyView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
    {
    this->m_SliceNavigationListener.RenderWindowPartDeactivated(renderWindowPart);
    }

 - View class must pass its on render window part in its CreateQtPartControl(QWidget* parent)

    this->m_SliceNavigationListener.RenderWindowPartActivated(this->GetRenderWindowPart());

 - View class must connect the SliceChanged signal of the listener as see fit.
*/
class MITK_QT_COMMON QmitkSliceNavigationListener : public QObject
  {
    Q_OBJECT

  public:
    QmitkSliceNavigationListener();
    ~QmitkSliceNavigationListener() override;

  signals:
    void SliceChanged();

  public slots:
    void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
    void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);

  protected slots:
      /** Overwrite function to implement the behavior on slice/time changes. */
      void OnSliceChangedDelayed();

  protected:

    /**	@brief Calls OnSliceChangedDelayed so the event isn't triggered multiple times. */
    void OnSliceChangedInternal(const itk::EventObject& e);

    void OnSliceNavigationControllerDeleted(const itk::Object* sender, const itk::EventObject& /*e*/);

    /** Initializes and sets the observers that are used to monitor changes in the selected position
    or time point in order to actualize the view.h*/
    bool InitObservers();
    void RemoveObservers(const mitk::SliceNavigationController* deletedSlicer);
    /** Removes all observers of the deletedPart. If null pointer is passed all observers will be removed.*/
    void RemoveAllObservers(mitk::IRenderWindowPart* deletedPart = nullptr);

    mitk::IRenderWindowPart* m_renderWindowPart;

    // Needed for observing the events for when a slice or time step is changed.
    bool m_PendingSliceChangedEvent;

    /**Helper structure to manage the registered observer events.*/
    struct ObserverInfo
    {
      mitk::SliceNavigationController* controller;
      int observerTag;
      std::string renderWindowName;
      mitk::IRenderWindowPart* renderWindowPart;

      ObserverInfo(mitk::SliceNavigationController* controller, int observerTag,
        const std::string& renderWindowName, mitk::IRenderWindowPart* part);
    };

    typedef std::multimap<const mitk::SliceNavigationController*, ObserverInfo> ObserverMapType;
    ObserverMapType m_ObserverMap;

    /**	@brief	Is a visualization currently running? */
    bool m_internalUpdateFlag;
  };

#endif

