/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkDicomInspectorView_h
#define QmitkDicomInspectorView_h

#include "ui_QmitkDicomInspectorViewControls.h"

// Blueberry
#include <berryIPartListener.h>

// mitk DICOMReader module
#include <mitkDICOMTag.h>
#include <mitkDICOMProperty.h>

// mitk gui common plugin
#include <mitkIRenderWindowPartListener.h>

// mitk gui qt common plugin
#include <QmitkAbstractView.h>
#include "QmitkSelectionServiceConnector.h"

/**
 *	@brief	View class defining the UI part of the ModelFitVisualization plug-in.
 */
class QmitkDicomInspectorView :
  public QmitkAbstractView,
  public mitk::IRenderWindowPartListener
{

  Q_OBJECT

public:

  QmitkDicomInspectorView();
  ~QmitkDicomInspectorView() override;

  static const std::string VIEW_ID;

  protected slots:

  /**
   *	@brief	Triggered when the voxel or time step selection changes.
   *			Calculates the curve and points for the current fit if the visualization is running.
   */
  void OnSliceChangedDelayed();

protected:

  void CreateQtPartControl(QWidget* parent) override;

  void SetFocus() override;

  /** @brief called by the selection widget when the selection has changed */
  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  /**	@brief Calls OnSliceChangedDelayed so the event isn't triggered multiple times. */
  void OnSliceChanged(const itk::EventObject& e);

  void OnSliceNavigationControllerDeleted(const itk::Object* sender, const itk::EventObject& /*e*/);

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  /** Initializes and sets the observers that are used to monitor changes in the selected position
   or time point in order to actualize the view.h*/
  bool InitObservers();
  void RemoveObservers(const mitk::SliceNavigationController* deletedSlicer);
  /** Removes all observers of the deletedPart. If null pointer is passed all observers will be removed.*/
  void RemoveAllObservers(mitk::IRenderWindowPart* deletedPart = nullptr);

  /** Sets m_currentSelectedPosition to the current selection and validates if this position is valid
   * for the input image of the currently selected fit. If it is valid, m_validSelectedPosition is set to true.
   * If the fit, his input image or geometry is not specified, it will also handled as invalid.*/
  void ValidateAndSetCurrentPosition();

  Ui::QmitkDicomInspectorViewControls m_Controls;
  mitk::IRenderWindowPart* m_renderWindowPart;

  std::unique_ptr<QmitkSelectionServiceConnector> m_SelectionServiceConnector;

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

  /** @brief currently valid selected position in the inspector*/
  mitk::Point3D m_currentSelectedPosition;
  /** @brief indicates if the currently selected position is valid for the currently selected fit.
   * This it is within the input image */
  bool m_validSelectedPosition;

  unsigned int m_currentSelectedTimeStep;

  itk::IndexValueType m_currentSelectedZSlice;

  /** @brief currently selected node for the DICOM information*/
  mitk::DataNode::ConstPointer m_SelectedNode;

  mitk::BaseData::ConstPointer m_SelectedData;
  /**	@brief	Is a visualization currently running? */
  bool m_internalUpdateFlag;

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

private:

  void SetAsSelectionListener(bool checked);

};

#endif // QmitkDicomInspectorView_h
