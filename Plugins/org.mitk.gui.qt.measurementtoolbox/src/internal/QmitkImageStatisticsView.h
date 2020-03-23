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
#include <mitkImageStatisticsContainer.h>
#include <QmitkNodeSelectionDialog.h>

#include <mitkPropertyRelations.h>

class QmitkImageStatisticsDataGenerator;
class QmitkDataGenerationJobBase;

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

  void UpdateIntensityProfile();
  void UpdateHistogramWidget();

  QmitkChartWidget::ColorTheme GetColorTheme() const;

  void ResetGUI();
  void ResetGUIDefault();

  void OnGenerationStarted();
  void OnGenerationFinished();
  void OnJobError(QString error, const QmitkDataGenerationJobBase* failedJob);
  void OnRequestHistogramUpdate(unsigned int);
  void OnCheckBoxIgnoreZeroStateChanged(int state);
  void OnButtonSelectionPressed();

  // member variable
  Ui::QmitkImageStatisticsViewControls m_Controls;

private:
  QmitkNodeSelectionDialog::SelectionCheckFunctionType CheckForSameGeometry() const;

  typedef itk::SimpleMemberCommand<QmitkImageStatisticsView> ITKCommandType;
  mitk::PlanarFigure::Pointer m_selectedPlanarFigure = nullptr;

  long m_PlanarFigureObserverTag;
  bool m_IgnoreZeroValueVoxel = false;
  std::vector<mitk::DataNode::ConstPointer> m_selectedMaskNodes;
  std::vector<mitk::DataNode::ConstPointer> m_selectedImageNodes;
  QmitkNodeSelectionDialog::NodeList m_SelectedNodeList;
  std::vector<mitk::ImageStatisticsContainer::ConstPointer> m_StatisticsForSelection;
  QmitkImageStatisticsDataGenerator* m_DataGenerator = nullptr;
};

#endif // QMITKIMAGESTATISTICSVIEW_H
