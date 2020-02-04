/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKIMAGESTATISTICSVIEW_H
#define QMITKIMAGESTATISTICSVIEW_H

#include "ui_QmitkImageStatisticsViewControls.h"

#include <QmitkAbstractView.h>
#include <QmitkImageStatisticsCalculationRunnable.h>
#include <mitkImageStatisticsContainer.h>
#include <QmitkNodeSelectionDialog.h>

#include <mitkPropertyRelations.h>

/*!
\brief QmitkImageStatisticsView is a bundle that allows statistics calculation from images. Three modes
are supported: 1. Statistics of one image, 2. Statistics of an image and a segmentation, 3. Statistics
of an image and a Planar Figure. The statistics calculation is realized in a separate thread to keep the
gui accessible during calculation.

\ingroup Plugins/org.mitk.gui.qt.measurementtoolbox
*/
class QmitkImageStatisticsView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /*!
  \brief default destructor */
  ~QmitkImageStatisticsView() override;
  /*!
  \brief Creates the widget containing the application controls, like sliders, buttons etc.*/
  void CreateQtPartControl(QWidget *parent) override;

protected:

  using HistogramType = mitk::ImageStatisticsContainer::HistogramType;

  void SetFocus() override { };

  virtual void CreateConnections();

  void CalculateOrGetMultiStatistics();
  void CalculateOrGetStatistics();
  void CalculateStatistics(const mitk::Image* image, const mitk::Image* mask = nullptr,
                           const mitk::PlanarFigure* maskPlanarFigure = nullptr);

  void ComputeAndDisplayIntensityProfile(mitk::Image * image, mitk::PlanarFigure* maskPlanarFigure);
  void FillHistogramWidget(const std::vector<const HistogramType*> &histogram,
                           const std::vector<std::string> &dataLabels);
  QmitkChartWidget::ColorTheme GetColorTheme() const;

  void ResetGUI();
  void ResetGUIDefault();

  void OnStatisticsCalculationEnds();
  void OnRequestHistogramUpdate(unsigned int);
  void OnCheckBoxIgnoreZeroStateChanged(int state);
  void OnButtonSelectionPressed();
  void OnDialogSelectionChanged();

  // member variable
  Ui::QmitkImageStatisticsViewControls m_Controls;

private:

  void HandleExistingStatistics(mitk::Image::ConstPointer, mitk::BaseData::ConstPointer,
                                mitk::ImageStatisticsContainer::Pointer);

  std::string GenerateStatisticsNodeName(mitk::Image::ConstPointer, mitk::BaseData::ConstPointer);

  void SetupRelationRules(mitk::Image::ConstPointer, mitk::BaseData::ConstPointer,
                          mitk::ImageStatisticsContainer::Pointer);

  mitk::DataNode::Pointer GetNodeForStatisticsContainer(mitk::ImageStatisticsContainer::ConstPointer container);

  QmitkNodeSelectionDialog::SelectionCheckFunctionType CheckForSameGeometry() const;
  bool CheckForSameGeometry(const mitk::DataNode* node1, const mitk::DataNode* node2) const;

  typedef itk::SimpleMemberCommand<QmitkImageStatisticsView> ITKCommandType;
  mitk::DataNode::ConstPointer m_selectedImageNode = nullptr;
  mitk::DataNode::ConstPointer m_selectedMaskNode = nullptr;
  mitk::PlanarFigure::Pointer m_selectedPlanarFigure = nullptr;

  QmitkNodeSelectionDialog* m_SelectionDialog = nullptr;

  long m_PlanarFigureObserverTag;
  bool m_ForceRecompute = false;
  bool m_IgnoreZeroValueVoxel = false;
  std::vector<mitk::DataNode::ConstPointer> m_selectedMaskNodes;
  std::vector<mitk::DataNode::ConstPointer> m_selectedImageNodes;
  std::vector<mitk::ImageStatisticsContainer::ConstPointer> m_StatisticsForSelection;
  std::vector<QmitkImageStatisticsCalculationRunnable*> m_Runnables;

};

#endif // QMITKIMAGESTATISTICSVIEW_H
