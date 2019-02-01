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

#ifndef QmitkImageStatisticsReloadedView_H__INCLUDED
#define QmitkImageStatisticsReloadedView_H__INCLUDED

#include "ui_QmitkImageStatisticsReloadedViewControls.h"

// Qmitk includes
#include <QmitkAbstractView.h>
#include <QmitkImageStatisticsCalculationJob.h>
#include <mitkImageStatisticsContainer.h>

#include <mitkILifecycleAwarePart.h>
#include <berryIPartListener.h>
#include <mitkPropertyRelations.h>

/*!
\brief QmitkImageStatisticsView is a bundle that allows statistics calculation from images. Three modes
are supported: 1. Statistics of one image, 2. Statistics of an image and a segmentation, 3. Statistics
of an image and a Planar Figure. The statistics calculation is realized in a separate thread to keep the
gui accessible during calculation.

\ingroup Plugins/org.mitk.gui.qt.measurementtoolbox
*/
class QmitkImageStatisticsReloadedView : public QmitkAbstractView, public mitk::ILifecycleAwarePart, public berry::IPartListener
{
  Q_OBJECT

public:
  using HistogramType = mitk::ImageStatisticsContainer::HistogramType;

  /*!
  \brief default constructor */
  QmitkImageStatisticsReloadedView(QObject *parent = nullptr, const char *name = nullptr);
  /*!
  \brief default destructor */
  virtual ~QmitkImageStatisticsReloadedView();
  /*!
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc. */
  virtual void CreateQtPartControl(QWidget *parent) override;
  /*!
  \brief method for creating the connections of main and control widget */
  virtual void CreateConnections();
  /*!
  \brief  Is called from the selection mechanism once the data manager selection has changed*/
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &selectedNodes) override;

  void PrepareDataStorageComboBoxes();

  static const std::string VIEW_ID;

  void FillHistogramWidget(const std::vector<HistogramType::ConstPointer>& histogram, const std::vector<std::string>& dataLabels);
  QmitkChartWidget::ColorTheme GetColorTheme() const;
protected:
  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

  virtual void SetFocus() override;

  /** \brief Is called right before the view closes (before the destructor) */
  virtual void PartClosed(const berry::IWorkbenchPartReference::Pointer&) override;
 
  /** \brief Required for berry::IPartListener */
  virtual Events::Types GetPartEventTypes() const override { return Events::CLOSED; }

  void OnImageSelectorChanged();
  void OnMaskSelectorChanged();

  void CalculateOrGetStatistics();

  void ComputeAndDisplayIntensityProfile(mitk::Image * image, mitk::PlanarFigure::Pointer maskPlanarFigure);

  void ResetGUI();

  void OnStatisticsCalculationEnds();
  void OnRequestHistogramUpdate(unsigned int nBins);
  void OnCheckBoxIgnoreZeroStateChanged(int state);
  void OnSliderWidgetHistogramChanged(double value);

  void CalculateStatistics(mitk::Image::ConstPointer image, mitk::Image::ConstPointer mask=nullptr, mitk::PlanarFigure::ConstPointer maskPlanarFigure = nullptr);
  
  // member variables
  Ui::QmitkImageStatisticsReloadedViewControls m_Controls;

private:
  typedef itk::SimpleMemberCommand< QmitkImageStatisticsReloadedView > ITKCommandType;
  QmitkImageStatisticsCalculationJob * m_CalculationThread = nullptr;
  bool m_StatisticsUpdatePending=false;
  mitk::DataNode::ConstPointer m_selectedImageNode = nullptr, m_selectedMaskNode = nullptr;

  std::vector<mitk::PropertyRelations::RuleResultVectorType> m_statisticContainerRules;
  mitk::PlanarFigure::Pointer m_selectedPlanarFigure=nullptr;
  long m_PlanarFigureObserverTag;
  bool m_ForceRecompute = false;
};
#endif // QmitkImageStatisticsView_H__INCLUDED
