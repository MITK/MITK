/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef ModelFitInspectorView_h
#define ModelFitInspectorView_h

// Blueberry
//#include <berryISelectionListener.h>
#include <berryIPartListener.h>

// mitk
#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include "QmitkSliceNavigationListener.h"

#include "mitkModelFitStaticParameterMap.h"
#include "mitkModelParameterizerBase.h"
#include "mitkModelFitInfo.h"
#include "mitkIModelFitProvider.h"
#include "mitkModelFitPlotDataHelper.h"
#include "QmitkSelectionServiceConnector.h"
#include "QmitkFitParameterModel.h"

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
  ~ModelFitInspectorView() override;

  static const std::string VIEW_ID;

protected slots:

  void OnSliceChanged();

  /**
   *	@brief			Triggered when the selection of the "Modelfit" combo box changes.
   *					Sets the selected fit as the current one.
   *	@param index	The index (in the combo box) of the selected item.
   */
  void OnFitSelectionChanged(int index);

  void OnInputChanged(const QList<mitk::DataNode::Pointer>& nodes);

  void OnPositionBookmarksChanged();

  /** Triggered when the selection of "fixed" y axis scaling changes*/
  void OnScaleFixedYChecked(bool checked);
  void OnScaleToDataYClicked();
  void OnFixedScalingYChanged(double value);


  /** Triggered when the selection of "fixed" x axis scaling changes*/
  void OnScaleFixedXChecked(bool checked);
  void OnScaleToDataXClicked();
  void OnFixedScalingXChanged(double value);

  void OnFullPlotClicked(bool checked);

protected:

  void CreateQtPartControl(QWidget* parent) override;

  void SetFocus() override;

  void NodeRemoved(const mitk::DataNode* node) override;

  /** Helper that actualizes the fit selection widget and returns the index of the currently selected
   * fit.*/
  int ActualizeFitSelectionWidget();

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

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
   * @brief Returns the input node that was used to generate the passed model fit node if it exists.
   * @param node Node that is a model fit result.
   * @return The input node that was used to generate the passed node. If the input node cannot
   * be found in the storage or the passed node is not a model fit result node, a nullptr will be returned.
   */
  mitk::DataNode::ConstPointer GetInputNode(mitk::DataNode::ConstPointer node);

  /** Sets m_currentSelectedPosition to the current selection and validates if this position is valid
   * for the input image of the currently selected fit. If it is valid, m_validSelectedPosition is set to true.
   * If the fit, his input image or geometry is not specified, it will also handled as invalid.*/
  void ValidateAndSetCurrentPosition();

  /** Returns the current input image. If a current fit is set it will be its input image.
   * Otherwise it will be the image stored in the currently selected node. If the node is not set, contains no image
   * or the image is not 4D, NULL will be returned.*/
  mitk::Image::ConstPointer GetCurrentInputImage() const;

  /** Returns the variable/time grid of the GetCurrentInputImage(). If a model fit is selected its provider will be used
   to get the correct grid, otherwise just a simple time grid will be extracted.*/
  const mitk::ModelBase::TimeGridType GetCurrentTimeGrid() const;

  Ui::ModelFitInspectorViewControls m_Controls;
  mitk::IRenderWindowPart* m_renderWindowPart;

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

  mitk::WeakPointer<mitk::DataNode> m_PositionBookmarksNode;
  mitk::WeakPointer<mitk::PointSet> m_PositionBookmarks;

  /** @brief	Number of interpolation steps between two x values */
  static const unsigned int INTERPOLATION_STEPS;

  /*************************************/
  /* Members for visualizing the model */

  itk::TimeStamp m_selectedNodeTime;
  itk::TimeStamp m_currentFitTime;
  itk::TimeStamp m_currentPositionTime;
  itk::TimeStamp m_lastRefreshTime;

  mitk::ModelFitPlotData m_PlotCurves;

  std::unique_ptr<QmitkSelectionServiceConnector> m_SelectionServiceConnector;
  QmitkFitParameterModel* m_FitParameterModel;

  QmitkSliceNavigationListener m_SliceChangeListener;

  /** Check and updates the plot data if needed.
  * @return indicates if something was refreshed (true)*/
  bool RefreshPlotData();

  void RenderPlot();
  void RenderPlotCurve(const mitk::PlotDataCurveCollection* curveCollection, const QColor& sampleColor, const QColor& signalColor, const std::string& posString);

  void RenderFitInfo();

  void EnsureBookmarkPointSet();

  static mitk::PlotDataCurveCollection::Pointer RefreshPlotDataCurveCollection(const mitk::Point3D& position,
    const mitk::Image* input, const mitk::modelFit::ModelFitInfo* fitInfo,
    const mitk::ModelBase::TimeGridType& timeGrid, mitk::ModelParameterizerBase* parameterizer);
};

#endif // ModelFitInspectorView_h
