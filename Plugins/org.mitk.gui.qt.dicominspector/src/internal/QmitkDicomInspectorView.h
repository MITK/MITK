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


#ifndef QmitkDicomInspectorView_h
#define QmitkDicomInspectorView_h

// Blueberry
//#include <berryISelectionListener.h>
#include <berryIPartListener.h>

// mitk
#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include <mitkDICOMTag.h>
#include <mitkDICOMProperty.h>

// Qt
#include "ui_QmitkDicomInspectorViewControls.h"


/**
 *	@brief	View class defining the UI part of the ModelFitVisualization plug-in.
 */
class QmitkDicomInspectorView :
  public QmitkAbstractView,
  public mitk::IRenderWindowPartListener
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  QmitkDicomInspectorView();
  ~QmitkDicomInspectorView();

  static const std::string VIEW_ID;

  protected slots:

  /**
   *	@brief	Triggered when the voxel or time step selection changes.
   *			Calculates the curve and points for the current fit if the visualization is running.
   */
  void OnSliceChangedDelayed();

protected:

  virtual void CreateQtPartControl(QWidget* parent);

  virtual void SetFocus();

  /** @brief called by QmitkFunctionality when DataManager's selection has changed */
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes);

  /**	@brief Calls OnSliceChangedDelayed so the event isn't triggered multiple times. */
  void OnSliceChanged(const itk::EventObject& e);

  void OnSliceNavigationControllerDeleted(const itk::Object* sender, const itk::EventObject& /*e*/);

  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);

  /** Initializes and sets the observers that are used to monitor changes in the selected position
   or time point in order to actualize the view.h*/
  bool InitObservers();
  void RemoveObservers(const mitk::SliceNavigationController* deletedSlicer);
  /** Removes all observers of the deletedPart. If null pointer is passed all observers will be removed.*/
  void RemoveAllObservers(mitk::IRenderWindowPart* deletedPart = NULL);

  /** Sets m_currentSelectedPosition to the current selection and validates if this position is valid
   * for the input image of the currently selected fit. If it is valid, m_validSelectedPosition is set to true.
   * If the fit, his input image or geometry is not specified, it will also handled as invalid.*/
  void ValidateAndSetCurrentPosition();

  Ui::DicomInspectorViewControls m_Controls;
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

  /** @brief currently valid selected position in the inspector*/
  mitk::Point3D m_currentSelectedPosition;
  /** @brief indicates if the currently selected position is valid for the currently selected fit.
   * This it is within the input image */
  bool m_validSelectedPosition;

  unsigned int m_currentSelectedTimeStep;

  itk::IndexValueType m_currentSelectedZSlice;

  /** @brief currently selected node for the visualization logic*/
  mitk::DataNode::ConstPointer m_currentSelectedNode;

  mitk::BaseData::ConstPointer m_currentSelectedData;

  /** @brief	Number of interpolation steps between two x values */
  static const unsigned int INTERPOLATION_STEPS;

  /*************************************/
  /* Members for visualizing the model */

  itk::TimeStamp m_selectedNodeTime;
  itk::TimeStamp m_currentFitTime;
  itk::TimeStamp m_currentPositionTime;
  itk::TimeStamp m_lastRefreshTime;

  /**Helper structure to manage the registered observer events.*/
  struct TagInfo
  {
    mitk::DICOMTag tag;
    mitk::DICOMProperty::ConstPointer prop;

    TagInfo(const mitk::DICOMTag& aTag, mitk::DICOMProperty::ConstPointer aProp) : tag(aTag), prop(aProp)
    {
    };
  };

  typedef std::map<std::string, TagInfo> TagMapType;
  TagMapType m_Tags;

  void RenderTable();

  /** (re)initializes the headers of the data table*/
  void UpdateData();
  void UpdateLabels();
};

#endif // QmitkDicomInspectorView_h
