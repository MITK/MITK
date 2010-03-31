/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2005/06/28 12:37:25 $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkSegmentationPostProcessing_h
#define QmitkSegmentationPostProcessing_h

#include "QmitkFunctionality.h"
#include "mitkDataStorage.h"
#include "mitkToolManager.h"
#include "mitkDataNodeSelection.h"

#include <QObject>

#include <berryISelectionListener.h>

/**
 * \brief Provide context-menus for segmentations in data manager
 *
 * This class is mainly meant as a helper to the org.mitk.gui.qt.segmentation
 * view. Independency of this class has been attempted, so it might
 * also be useful in a different context.
*/
class QmitkSegmentationPostProcessing : public QObject
{
  Q_OBJECT
  
  public:
    
    // construction/destruction
    QmitkSegmentationPostProcessing(mitk::DataStorage* storage, QmitkFunctionality* functionality, QObject* parent = 0);
    virtual ~QmitkSegmentationPostProcessing();

    // for receiving messages
    void OnSurfaceCalculationDone();
    void OnThresholdingToolManagerToolModified();
    void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);

  protected slots:

    // reactions to context menu items
    void ThresholdImage(bool);
    void CreateSurface(bool);
    void CreateSmoothedSurface(bool);
    void ImageStatistics(bool);
    void AutocropSelected(bool);    

    // class internal slots
    void ThresholdingDone(int);
    void InternalCreateSurface(bool smoothed);

  protected:

    typedef std::vector<mitk::DataNode*> NodeList;
    NodeList GetSelectedNodes() const;

    mitk::Image::Pointer IncreaseCroppedImageSize( mitk::Image::Pointer image );

    // variables
    QmitkFunctionality* m_BlueBerryView;
    mitk::DataStorage::Pointer m_DataStorage;

    QAction* m_CreateSurfaceAction;
    QAction* m_CreateSmoothSurfaceAction;
    QAction* m_StatisticsAction;
    QAction* m_AutocropAction;
    QAction* m_ThresholdAction;

    QDialog* m_ThresholdingDialog;
    mitk::ToolManager::Pointer m_ThresholdingToolManager;
    
    berry::ISelectionListener::Pointer m_SelectionListener;
    mitk::DataNodeSelection::ConstPointer m_CurrentSelection;
    friend struct berry::SelectionChangedAdapter<QmitkSegmentationPostProcessing>;
};

#endif

