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


#ifndef ModelFitInspectorView_h
#define ModelFitInspectorView_h

// Blueberry
//#include <berryISelectionListener.h>
#include <berryIPartListener.h>

// mitk
#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include "mitkModelFitStaticParameterMap.h"
#include "mitkModelParameterizerBase.h"
#include "mitkModelFitInfo.h"
#include "mitkIModelFitProvider.h"

// Qt
#include "ui_ModelFitInspectorViewControls.h"


/**
 *	@brief	View class defining the UI part of the ModelFitInspector plug-in.
 */
class ModelFitInspectorView :
  public QmitkAbstractView,
  public mitk::IRenderWindowPartListener
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  ModelFitInspectorView();
  ~ModelFitInspectorView();

  static const std::string VIEW_ID;

protected slots:

  /**
   *	@brief			Triggered when the selection of the "Modelfit" combo box changes.
   *					Sets the selected fit as the current one.
   *	@param index	The index (in the combo box) of the selected item.
   */
  void OnFitSelectionChanged(int index);

  /**
   *	@brief	Triggered when the voxel or time step selection changes.
   *			Calculates the curve and points for the current fit if the visualization is running.
   */
  void OnSliceChangedDelayed();

  /** Triggered when the selection of "fixed" y axis scaling changes*/
  void OnScaleFixedYChecked(bool checked);
  void OnScaleToDataYClicked();
  void OnFixedScalingYChanged(double value);


  /** Triggered when the selection of "fixed" x axis scaling changes*/
  void OnScaleFixedXChecked(bool checked);
  void OnScaleToDataXClicked();
  void OnFixedScalingXChanged(double value);

  void OnExportClicked();


  /** @brief Saves the results table to clipboard */
  void OnClipboardResultsButtonClicked();


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

  /**
   *	@brief			Calculates the curve data using the current fit's model and parameterizer.
   *	@param position	Indicating the point in the input image for which the model curve should be calculated.
   *	@return			A vector of points for the curve data.
   */
  QmitkPlotWidget::XYDataVector CalcCurveFromModel(const mitk::Point3D& position);

  /**
   *	@brief			Calculates the curve data using the current fit's function string.
   *	@param timeGrid	The time grid containing the (interpolated) x values for the curve data.
   *	@return			A vector of points for the curve data.
   */
  QmitkPlotWidget::XYDataVector CalcCurveFromFunction(const mitk::Point3D& position,
      const mitk::ModelBase::TimeGridType&
      timeGrid);

  /**
   *	@brief		Returns the parent node of the given node if it exists.
   *	@param node	The node whose parent node should be returned.
   *	@return		The parent node of the given node or NULL if it doesn't exist.
   */
  mitk::DataNode::ConstPointer GetParentNode(mitk::DataNode::ConstPointer node);

  /** Super sample passed time grid with the factor INTERPOLATION_STEPS and interpolates linear in between.*/
  mitk::ModelBase::TimeGridType GenerateInterpolatedTimeGrid(const mitk::ModelBase::TimeGridType&
      grid) const;

  /** Sets m_currentSelectedPosition to the current selection and validates if this position is valid
   * for the input image of the currently selected fit. If it is valid, m_validSelectedPosition is set to true.
   * If the fit, his input image or geometry is not specified, it will also handled as invalid.*/
  void ValidateAndSetCurrentPosition();

  /** Returns the current input image. If a current fit is set it will be its input image.
   * Otherwise it will be the image stored in the currently selected node. If the node is not set, contains no image
   * or the image is not 4D, NULL will be returned.*/
  mitk::Image::Pointer GetCurrentInputImage() const;

  /** Returns the variable/time grid of the GetCurrentInputImage(). If a model fit is selected its provider will be used
   to get the correct grid, otherwise just a simple time grid will be extracted.*/
  const mitk::ModelBase::TimeGridType GetCurrentTimeGrid() const;

  Ui::ModelFitInspectorViewControls m_Controls;
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
  /**	@brief	List of modelfits currently in the data manager */
  typedef std::map<const mitk::modelFit::ModelFitInfo::UIDType, mitk::modelFit::ModelFitInfo::ConstPointer>
  ModelFitInfoListType;
  ModelFitInfoListType m_modelfitList;
  /** @brief	The currently selected modelfit */
  mitk::modelFit::ModelFitInfo::ConstPointer m_currentFit;

  /**	@brief	Pointer to the instance of the model parameterizer for the current fit */
  mitk::ModelParameterizerBase::Pointer m_currentModelParameterizer;
  mitk::IModelFitProvider* m_currentModelProviderService;

  /** @brief currently valid selected position in the inspector*/
  mitk::Point3D m_currentSelectedPosition;
  /** @brief indicates if the currently selected position is valid for the currently selected fit.
   * This it is within the input image */
  bool m_validSelectedPosition;

  /** @brief currently selected time step of the selected node for the visualization logic*/
  unsigned int m_currentSelectedTimeStep;

  /** @brief currently selected node for the visualization logic*/
  mitk::DataNode::ConstPointer m_currentSelectedNode;

  /** @brief	Number of interpolation steps between two x values */
  static const unsigned int INTERPOLATION_STEPS;

  /*************************************/
  /* Members for visualizing the model */

  itk::TimeStamp m_selectedNodeTime;
  itk::TimeStamp m_currentFitTime;
  itk::TimeStamp m_currentPositionTime;
  itk::TimeStamp m_lastRefreshTime;

  QmitkPlotWidget::XYDataVector m_ImagePlotCurve;
  QmitkPlotWidget::XYDataVector m_ModelPlotCurve;

  typedef std::map<std::string, QmitkPlotWidget::XYDataVector> CurveMapType;
  CurveMapType m_InputDataPlotCurves;

  /** Check and updates the plot data if needed.
  * @return indicates if something was refreshed (true)*/
  bool RefreshPlotData();


  void RenderPlot();
  void RenderFitInfo();

  /** (re)initializes the headers of the data table*/
  void InitDataTable();


};

#endif // ModelFitInspectorView_h
