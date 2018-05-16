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

#ifndef QmitkImageStatisticsView_H__INCLUDED
#define QmitkImageStatisticsView_H__INCLUDED

#include "ui_QmitkImageStatisticsViewControls.h"

// Qmitk includes
#include <QmitkAbstractView.h>
#include <QmitkStepperAdapter.h>
#include "QmitkImageStatisticsCalculationThread.h"
#include <berryIPartListener.h>

// mitk includes
#include <mitkILifecycleAwarePart.h>
#include <mitkPlanarLine.h>
#include <mitkIntensityProfile.h>

#include <berryIPreferences.h>

/*!
\brief QmitkImageStatisticsView is a bundle that allows statistics calculation from images. Three modes
are supported: 1. Statistics of one image, 2. Statistics of an image and a segmentation, 3. Statistics
of an image and a Planar Figure. The statistics calculation is realized in a separate thread to keep the
gui accessible during calculation.

\ingroup Plugins/org.mitk.gui.qt.measurementtoolbox
*/
class QmitkImageStatisticsView : public QmitkAbstractView, public mitk::ILifecycleAwarePart, public berry::IPartListener
{
  Q_OBJECT

private:
  /*!
  \  Convenient typedefs */
  typedef QList<mitk::DataNode*> SelectedDataNodeVectorType;
  typedef itk::SimpleMemberCommand< QmitkImageStatisticsView > ITKCommandType;

  std::map<double, double> ConvertHistogramToMap(itk::Statistics::Histogram<double>::ConstPointer histogram) const;
  std::vector<double> ConvertIntensityProfileToVector(mitk::IntensityProfile::ConstPointer intensityProfile) const;
  std::vector<QString> AssembleStatisticsIntoVector(mitk::ImageStatisticsCalculator::StatisticsContainer::ConstPointer statistics, mitk::Image::ConstPointer image, bool noVolumeDefined=false) const;

  QString GetFormattedIndex(const vnl_vector<int>& vector) const;
  QString GetFormattedString(double value, unsigned int decimals) const;
public:

  /*!
  \brief default constructor */
  QmitkImageStatisticsView(QObject *parent = nullptr, const char *name = nullptr);
  /*!
  \brief default destructor */
  virtual ~QmitkImageStatisticsView();
  /*!
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc. */
  virtual void CreateQtPartControl(QWidget *parent) override;
  /*!
  \brief method for creating the connections of main and control widget */
  virtual void CreateConnections();
  /*!
  \brief  Is called from the selection mechanism once the data manager selection has changed*/
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &selectedNodes) override;

  static const std::string VIEW_ID;
  static const int STAT_TABLE_BASE_HEIGHT;

  public slots:
  /** \brief  Called when the statistics update is finished, sets the results to GUI.*/
  void OnThreadedStatisticsCalculationEnds();

  /** \brief Update bin size for histogram resolution. */
  void OnHistogramNBinsCheckBoxValueChanged();

  protected slots:
  /** \brief  Saves the histogram to the clipboard */
  void OnClipboardHistogramButtonClicked();
  /** \brief  Saves the statistics to the clipboard */
  void OnClipboardStatisticsButtonClicked();
  /** \brief  Indicates if zeros should be excluded from statistics calculation */
  void OnIgnoreZerosCheckboxClicked();
  /** \brief Checks if update is possible and calls StatisticsUpdate() possible */
  void RequestStatisticsUpdate();
  /** \brief Jump to coordinates stored in the double clicked cell */
  void JumpToCoordinates(int row, int col);
  /** \brief Toogle GUI elements if histogram default bin size checkbox value changed. */
  void OnDefaultNBinsSpinBoxChanged();

  void OnShowSubchartBoxChanged();

  void OnBarRadioButtonSelected();

  void OnLineRadioButtonSelected();

  void OnPageSuccessfullyLoaded();

signals:
  /** \brief Method to set the data to the member and start the threaded statistics update */
  void StatisticsUpdate();

protected:
  /** \brief  Writes the calculated statistics to the GUI */
  void FillStatisticsTableView(const std::vector<mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer> &statistics,
    const mitk::Image *image);

  void FillLinearProfileStatisticsTableView(mitk::ImageStatisticsCalculator::StatisticsContainer::ConstPointer statistics, const mitk::Image *image);

  /** \brief  Removes statistics from the GUI */
  void InvalidateStatisticsTableView();

  /** \brief Recalculate statistics for currently selected image and mask and
  * update the GUI. */
  void UpdateStatistics();

  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

  virtual void SetFocus() override;

  /** \brief Method called when itkModifiedEvent is called by selected data. */
  void SelectedDataModified();
  /** \brief  Method called when the data manager selection changes */
  void SelectionChanged(const QList<mitk::DataNode::Pointer> &selectedNodes);

  void DisableHistogramGUIElements();

  void ResetHistogramGUIElementsToDefault();

  void EnableHistogramGUIElements();

  /** \brief  Method called to remove old selection when a new selection is present */
  void ReinitData();
  /** \brief  writes the statistics to the gui*/
  void WriteStatisticsToGUI();

  void NodeRemoved(const mitk::DataNode *node) override;

  /** \brief Is called right before the view closes (before the destructor) */
  virtual void PartClosed(const berry::IWorkbenchPartReference::Pointer&) override;
  /** \brief Is called from the image navigator once the time step has changed */
  void OnTimeChanged(const itk::EventObject&);
  /** \brief Required for berry::IPartListener */
  virtual Events::Types GetPartEventTypes() const override { return Events::CLOSED; }

  // member variables
  Ui::QmitkImageStatisticsViewControls *m_Controls;
  // if you have a planar figure selected, the statistics values will be saved in this one.
  std::vector<QString> m_PlanarFigureStatistics;
  QmitkImageStatisticsCalculationThread* m_CalculationThread;

  // Image and mask data
  mitk::Image* m_SelectedImage;
  mitk::Image* m_SelectedImageMask;
  mitk::PlanarFigure* m_SelectedPlanarFigure;

  // observer tags
  long m_ImageObserverTag;
  long m_ImageMaskObserverTag;
  long m_PlanarFigureObserverTag;
  long m_TimeObserverTag;

  SelectedDataNodeVectorType m_SelectedDataNodes;

  bool m_CurrentStatisticsValid;
  bool m_StatisticsUpdatePending;
  bool m_DataNodeSelectionChanged;
  bool m_Visible;

  unsigned int m_HistogramNBins;

  std::vector<mitk::Point3D>     m_WorldMinList;
  std::vector<mitk::Point3D>     m_WorldMaxList;

  std::vector<double> m_IntensityProfileList;
  berry::IPreferences::Pointer m_StylePref;
};
#endif // QmitkImageStatisticsView_H__INCLUDED
