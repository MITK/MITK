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


#ifndef __Q_MITK_MATCHPOINT_REGISTRATION_EVALUATOR_H
#define __Q_MITK_MATCHPOINT_REGISTRATION_EVALUATOR_H

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include "ui_QmitkMatchPointRegistrationEvaluator.h"


class QmitkSliceChangedListener : public QObject
  {
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

  public:
    QmitkSliceChangedListener();
    virtual ~QmitkSliceChangedListener();

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
    void RemoveAllObservers(mitk::IRenderWindowPart* deletedPart = NULL);

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

/*!
\brief QmitkMatchPointRegistrationEvaluator

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkMatchPointRegistrationEvaluator : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:  

  static const std::string VIEW_ID;

  /**
  * Creates smartpointer typedefs
  */
  berryObjectMacro(QmitkMatchPointRegistrationEvaluator)

    QmitkMatchPointRegistrationEvaluator();

  virtual void CreateQtPartControl(QWidget *parent);

  protected slots:

    /// \brief Called when the user clicks the GUI button

    void OnComboStyleChanged(int);
    void OnBlend50Pushed();
    void OnBlendTargetPushed();
    void OnBlendMovingPushed();
    void OnBlendTogglePushed();
    void OnSlideBlendChanged(int);
    void OnSpinBlendChanged(int);
    void OnSpinCheckerChanged(int);
    void OnWipeStyleChanged();
    void OnContourStyleChanged();

    void OnSliceChanged();

protected:
  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes);

  virtual void SetFocus();

  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);

  Ui::MatchPointRegistrationEvaluatorControls m_Controls;

private:
  QWidget *m_Parent;

  void Error(QString msg);

  /** Methods returns a list of all eval nodes in the data manager.*/
  QList<mitk::DataNode::Pointer> GetEvalNodes();

  /**
  * Updates the state of controls regarding to selected eval object.*/
  void ConfigureControls();

  mitk::DataNode::Pointer m_selectedEvalNode;
  mitk::DataStorage::SetOfObjects::ConstPointer m_evalNodes;

  QmitkSliceChangedListener m_SliceChangeListener;

  bool m_internalBlendUpdate;

  itk::TimeStamp m_selectedNodeTime;
  itk::TimeStamp m_currentPositionTime;

  /** @brief currently valid selected position in the inspector*/
  mitk::Point3D m_currentSelectedPosition;
  /** @brief indicates if the currently selected position is valid for the currently selected fit.
  * This it is within the input image */
  unsigned int m_currentSelectedTimeStep;
};

#endif // MatchPoint_h

