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
#include "QmitkStepperAdapter.h"
#include "QmitkImageStatisticsCalculationThread.h"

// mitk includes
#include "mitkImageStatisticsCalculator.h"
#include "mitkILifecycleAwarePart.h"
#include "mitkPlanarLine.h"

/*!
\brief QmitkImageStatisticsView is a bundle that allows statistics calculation from images. Three modes
       are supported: 1. Statistics of one image, 2. Statistics of an image and a segmentation, 3. Statistics
       of an image and a Planar Figure. The statistics calculation is realized in a seperate thread to keep the
       gui accessable during calculation.

\ingroup Plugins/org.mitk.gui.qt.measurementtoolbox
*/
class QmitkImageStatisticsView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  Q_OBJECT

private:

  /*!
  \  Convenient typedefs */
  typedef mitk::DataStorage::SetOfObjects  ConstVector;
  typedef ConstVector::ConstPointer        ConstVectorPointer;
  typedef ConstVector::ConstIterator       ConstVectorIterator;
  typedef std::map< mitk::Image *, mitk::ImageStatisticsCalculator::Pointer > ImageStatisticsMapType;
  typedef QList<mitk::DataNode*> SelectedDataNodeVectorType;
  typedef itk::SimpleMemberCommand< QmitkImageStatisticsView > ITKCommandType;

public:

  /*!
  \brief default constructor */
  QmitkImageStatisticsView(QObject *parent=0, const char *name=0);
  /*!
  \brief default destructor */
  virtual ~QmitkImageStatisticsView();
  /*!
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc. */
  virtual void CreateQtPartControl(QWidget *parent);
  /*!
  \brief method for creating the connections of main and control widget */
  virtual void CreateConnections();
  /*!
  \brief  not implemented*/
  //bool IsExclusiveFunctionality() const;
  /*!
  \brief  Is called from the selection mechanism once the data manager selection has changed*/
  void OnSelectionChanged( berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes );

  static const std::string VIEW_ID;

public slots:
    /** \brief  Called when the statistics update is finished, sets the results to GUI.*/
    void OnThreadedStatisticsCalculationEnds();

protected slots:
  /** \brief  Saves the histogram to the clipboard */
  void OnClipboardHistogramButtonClicked();
  /** \brief  Saves the statistics to the clipboard */
  void OnClipboardStatisticsButtonClicked();
  /** \brief  Indicates if zeros should be excluded from statistics calculation */
  void OnIgnoreZerosCheckboxClicked(  );
  /** \brief Checks if update is possible and calls StatisticsUpdate() possible */
  void RequestStatisticsUpdate();
  /** \brief Jump to coordinates stored in the double clicked cell */
  void JumpToCoordinates(int row, int col);

signals:
  /** \brief Method to set the data to the member and start the threaded statistics update */
  void StatisticsUpdate();

protected:
  /** \brief  Writes the calculated statistics to the GUI */
  void FillStatisticsTableView( const mitk::ImageStatisticsCalculator::Statistics &s,
    const mitk::Image *image );
  /** \brief  Removes statistics from the GUI */
  void InvalidateStatisticsTableView();

  /** \brief Recalculate statistics for currently selected image and mask and
   * update the GUI. */
  void UpdateStatistics();

  /** \brief Listener for progress events to update progress bar. */
  void UpdateProgressBar();

  /** \brief Removes any cached images which are no longer referenced elsewhere. */
  void RemoveOrphanImages();

  /** \brief Computes an Intensity Profile along line and updates the histogram widget with it. */
  void ComputeIntensityProfile( mitk::PlanarLine* line );

  /** \brief Removes all Observers to images, masks and planar figures and sets corresponding members to zero */
  void ClearObservers();

  void Activated();
  void Deactivated();
  void Visible();
  void Hidden();
  void SetFocus();

  /** \brief Method called when itkModifiedEvent is called by selected data. */
  void SelectedDataModified();
  /** \brief  Method called when the data manager selection changes */
  void SelectionChanged(const QList<mitk::DataNode::Pointer> &selectedNodes);
  /** \brief  Method called to remove old selection when a new selection is present */
  void ReinitData();
  /** \brief  writes the statistics to the gui*/
  void WriteStatisticsToGUI();

  void NodeRemoved(const mitk::DataNode *node);

  // member variables
  Ui::QmitkImageStatisticsViewControls *m_Controls;
  QmitkImageStatisticsCalculationThread* m_CalculationThread;

  QmitkStepperAdapter*      m_TimeStepperAdapter;
  unsigned int              m_CurrentTime;
  QString                   m_Clipboard;

  // Image and mask data
  mitk::Image* m_SelectedImage;
  mitk::Image* m_SelectedImageMask;
  mitk::PlanarFigure* m_SelectedPlanarFigure;

  // observer tags
  long m_ImageObserverTag;
  long m_ImageMaskObserverTag;
  long m_PlanarFigureObserverTag;

  SelectedDataNodeVectorType m_SelectedDataNodes;

  bool m_CurrentStatisticsValid;
  bool m_StatisticsUpdatePending;
  bool m_StatisticsIntegrationPending;
  bool m_DataNodeSelectionChanged;
  bool m_Visible;

  mitk::Point3D     m_WorldMin;
  mitk::Point3D     m_WorldMax;
};
#endif // QmitkImageStatisticsView_H__INCLUDED
