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

// Qmitk includes
#include <QmitkAbstractView.h>
#include <QmitkImageStatisticsCalculationJob.h>
#include <mitkImageStatisticsContainer.h>
#include <QmitkNodeSelectionDialog.h>

#include <berryIPartListener.h>
#include <mitkPropertyRelations.h>
#include <QmitkImageStatisticsCalculationRunnable.h>

/*!
\brief QmitkImageStatisticsView is a bundle that allows statistics calculation from images. Three modes
are supported: 1. Statistics of one image, 2. Statistics of an image and a segmentation, 3. Statistics
of an image and a Planar Figure. The statistics calculation is realized in a separate thread to keep the
gui accessible during calculation.

\ingroup Plugins/org.mitk.gui.qt.measurementtoolbox
*/
class QmitkImageStatisticsView : public QmitkAbstractView, public berry::IPartListener
{
  Q_OBJECT

public:
  /*!
  \brief default constructor */
  QmitkImageStatisticsView(QObject *parent = nullptr, const char *name = nullptr);
  /*!
  \brief default destructor */
  ~QmitkImageStatisticsView() override;
  /*!
  \brief method for creating the widget containing the application controls, like sliders, buttons etc. */
  void CreateQtPartControl(QWidget *parent) override;
  /*!
  \brief  Is called from the selection mechanism once the data manager selection has changed*/
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &selectedNodes) override;

  static const std::string VIEW_ID;

protected:

  using HistogramType = mitk::ImageStatisticsContainer::HistogramType;

  void SetFocus() override;

  /** \brief Is called right before the view closes (before the destructor) */
  void PartClosed(const berry::IWorkbenchPartReference::Pointer&) override;

  /** \brief Required for berry::IPartListener */
  Events::Types GetPartEventTypes() const override { return Events::CLOSED; }

  void CalculateOrGetMultiStatistics();
  void CalculateOrGetStatistics();
  void CalculateStatistics(const mitk::Image* image,
                           const mitk::Image* mask = nullptr,
                           const mitk::PlanarFigure* maskPlanarFigure = nullptr);

  void ComputeAndDisplayIntensityProfile(mitk::Image * image, mitk::PlanarFigure* maskPlanarFigure);
  void FillHistogramWidget(const std::vector<const HistogramType*> &histogram,
                           const std::vector<std::string> &dataLabels);
  QmitkChartWidget::ColorTheme GetColorTheme() const;

  void ResetGUI();
  void ResetGUIDefault();

  /*!
  \brief method for creating the connections of main and control widget */
  virtual void CreateConnections();

  void OnStatisticsCalculationEnds();
  void OnRequestHistogramUpdate(unsigned int nBins);
  void OnCheckBoxIgnoreZeroStateChanged(int state);
  void OnButtonSelectionPressed();
  void OnDialogSelectionChanged();

  // member variable
  Ui::QmitkImageStatisticsViewControls m_Controls;

private:

  std::string GenerateStatisticsNodeName();

  void HandleExistingStatistics(mitk::Image::ConstPointer image,
                                mitk::BaseData::ConstPointer mask,
                                mitk::ImageStatisticsContainer::Pointer);

  void SetupRelationRules(mitk::ImageStatisticsContainer::Pointer, mitk::BaseData::ConstPointer mask);

  mitk::DataNode::Pointer GetNodeForStatisticsContainer(mitk::ImageStatisticsContainer::ConstPointer container);

  typedef itk::SimpleMemberCommand<QmitkImageStatisticsView> ITKCommandType;
  QmitkImageStatisticsCalculationJob* m_CalculationJob = nullptr;
  mitk::DataNode::ConstPointer m_selectedImageNode = nullptr;
  mitk::DataNode::ConstPointer m_selectedMaskNode = nullptr;
  mitk::PlanarFigure::Pointer m_selectedPlanarFigure = nullptr;

  QmitkNodeSelectionDialog* m_SelectionDialog = nullptr;

  long m_PlanarFigureObserverTag;
  bool m_ForceRecompute = false;
  std::vector<mitk::DataNode::ConstPointer> m_selectedMaskNodes;
  std::vector<mitk::DataNode::ConstPointer> m_selectedImageNodes;
  std::vector<mitk::ImageStatisticsContainer::ConstPointer> m_StatisticsForSelection;
  std::vector<QmitkImageStatisticsCalculationRunnable*> m_Runnables;

};

#endif // QMITKIMAGESTATISTICSVIEW_H
