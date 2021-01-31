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
#include <QmitkSliceNavigationListener.h>

#include <mitkPropertyRelations.h>

#include <mitkIRenderWindowPartListener.h>

class QmitkImageStatisticsDataGenerator;
class QmitkDataGenerationJobBase;

/*!
\brief QmitkImageStatisticsView is a bundle that allows statistics calculation from images. Three modes
are supported: 1. Statistics of one image, 2. Statistics of an image and a segmentation, 3. Statistics
of an image and a Planar Figure. The statistics calculation is realized in a separate thread to keep the
gui accessible during calculation.

\ingroup Plugins/org.mitk.gui.qt.measurementtoolbox
*/
class QmitkImageStatisticsView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /*!
  \brief default destructor */
  ~QmitkImageStatisticsView() override;

protected:
  /*!
  \brief Creates the widget containing the application controls, like sliders, buttons etc.*/
  void CreateQtPartControl(QWidget* parent) override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  using HistogramType = mitk::ImageStatisticsContainer::HistogramType;

  void SetFocus() override { };

  virtual void CreateConnections();

  void UpdateIntensityProfile();
  void UpdateHistogramWidget();

  QmitkChartWidget::ColorTheme GetColorTheme() const;

  void ResetGUI();

  void OnGenerationStarted(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, const QmitkDataGenerationJobBase* job);
  void OnGenerationFinished();
  void OnJobError(QString error, const QmitkDataGenerationJobBase* failedJob);
  void OnRequestHistogramUpdate(unsigned int);
  void OnCheckBoxIgnoreZeroStateChanged(int state);
  void OnButtonSelectionPressed();
  void OnImageSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList nodes);
  void OnROISelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList nodes);
  void OnSelectedTimePointChanged(const mitk::TimePointType& newTimePoint);

  // member variable
  Ui::QmitkImageStatisticsViewControls m_Controls;

private:
  QmitkNodeSelectionDialog::SelectionCheckFunctionType CheckForSameGeometry() const;
  mitk::NodePredicateBase::Pointer GenerateROIPredicate() const;

  std::vector<mitk::ImageStatisticsContainer::ConstPointer> m_StatisticsForSelection;
  QmitkImageStatisticsDataGenerator* m_DataGenerator = nullptr;

  QmitkSliceNavigationListener m_TimePointChangeListener;

};

#endif // QMITKIMAGESTATISTICSVIEW_H
