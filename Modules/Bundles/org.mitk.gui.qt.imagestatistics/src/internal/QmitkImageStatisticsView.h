/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 20:08:26 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 10185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QmitkImageStatisticsView_H__INCLUDED)
#define QmitkImageStatisticsView_H__INCLUDED

#include "QmitkFunctionality.h"
#include "../ImageStatisticsDll.h"
#include "ui_QmitkImageStatisticsViewControls.h"

#include "QmitkStepperAdapter.h"

#include "mitkImageStatisticsCalculator.h"

#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <itkTimeStamp.h>
#include "mitkPlanarLine.h"



/*!
\brief QmitkImageStatistics 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class IMAGESTATISTICS_EXPORT QmitkImageStatistics : public QObject, public QmitkFunctionality
{  
  Q_OBJECT

public:  

  /*! 
  \  Convenient typedefs
  */
  typedef mitk::DataStorage::SetOfObjects  ConstVector;
  typedef ConstVector::ConstPointer        ConstVectorPointer;
  typedef ConstVector::ConstIterator       ConstVectorIterator;

  /*!  
  \brief default constructor  
  */  
  QmitkImageStatistics(QObject *parent=0, const char *name=0);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkImageStatistics();

  /*!  
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
  */  
  virtual void CreateQtPartControl(QWidget *parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  bool IsExclusiveFunctionality() const;

  virtual bool event( QEvent *event );
  
protected slots:  
  /*
  * When an image is selected through the data tree combo box.
  */
  void onImageSelected(const mitk::DataTreeNode* item);
  void onImageSelected2(const mitk::DataTreeNode* item);

  void UpdateTimestep();

  void ClipboardHistogramButtonClicked();

  void ClipboardStatisticsButtonClicked();

  void ComputeStatistics(bool appendSourceData=false, bool toClipboard=false);

  void ComputeStatistics(mitk::Image::Pointer im1, mitk::Image::Pointer im2, bool appendSourceData, bool toClipboard);

  void ComputeStatisticsAllImages(bool appendSourceData=true);

  void ClipboardToFile(int count, std::string imageName1, std::string imageName2, std::string filename, bool appendToFile, bool matlab );

protected: 
  void UpdateCurrentSelection();

  void StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget );

  void FillStatisticsTableView( const mitk::ImageStatisticsCalculator::Statistics &s,
    const mitk::Image *image );

  void InvalidateStatisticsTableView();

  /** \brief Invoked when the DataManager selection changed */
  virtual void SelectionChanged(berry::IWorkbenchPart::Pointer part,
    berry::ISelection::ConstPointer selection);

  /** \brief Issues a request to update statistics by sending an event to the
  * Qt event processing queue.
  * 
  * Statistics update should only be executed after program execution returns
  * to the Qt main loop. This mechanism also prevents multiple execution of
  * updates where only one is required.*/
  void RequestStatisticsUpdate();

  /** \brief Recalculate statistics for currently selected image and mask and
   * update the GUI. */
  void UpdateStatistics();

  /** \brief Listener for progress events to update progress bar. */
  void UpdateProgressBar();


  /** \brief Removes any cached images which are no longer referenced elsewhere. */
  void RemoveOrphanImages();

  /** \brief Computes an Intensity Profile along line and updates the histogram widget with it. */
  void ComputeIntensityProfile( mitk::PlanarLine* line );



  typedef std::map< mitk::Image *, mitk::ImageStatisticsCalculator::Pointer >
    ImageStatisticsMapType;

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  Ui::QmitkImageStatisticsViewControls *m_Controls;

  QmitkStepperAdapter*      m_TimeStepperAdapter;
  unsigned int              m_CurrentTime;

  QString                     m_Clipboard;


  // Selection service
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;
  berry::ISelectionListener::Pointer m_SelectionListener;

  // Image and mask data
  mitk::DataTreeNode *m_SelectedImageNode;
  mitk::Image *m_SelectedImage;

  mitk::DataTreeNode *m_SelectedMaskNode;
  mitk::Image *m_SelectedImageMask;
  mitk::PlanarFigure *m_SelectedPlanarFigure;

  long m_ImageObserverTag;
  long m_ImageMaskObserverTag;
  long m_PlanarFigureObserverTag;

  // Hash map for associating one image statistics calculator with each iamge
  // (so that previously calculated histograms / statistics can be recovered
  // if a recalculation is not required)
  ImageStatisticsMapType m_ImageStatisticsMap;

  mitk::ImageStatisticsCalculator::Pointer m_CurrentStatisticsCalculator;
  
  bool m_CurrentStatisticsValid;

  bool m_StatisticsUpdatePending;
};


class QmitkRequestStatisticsUpdateEvent : public QEvent
{
public:
  enum Type
  {
    StatisticsUpdateRequest = QEvent::MaxUser - 1025
  };

  QmitkRequestStatisticsUpdateEvent()
    : QEvent( (QEvent::Type) StatisticsUpdateRequest ) {};
};

#endif // !defined(QmitkImageStatistics_H__INCLUDED)
