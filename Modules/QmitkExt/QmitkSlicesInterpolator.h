/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (Fr, 12 Sep 2008) $
Version:   $Revision: 15236 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkSlicesInterpolator_h_Included
#define QmitkSlicesInterpolator_h_Included

#include "mitkSliceNavigationController.h"
#include "mitkSegmentationInterpolationController.h"
#include "mitkDataTreeNode.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"

#include <QWidget>
#include <map>

namespace mitk
{
  class ToolManager;
  class PlaneGeometry;
}

class QmitkStdMultiWidget;
class QPushButton;

/**
  \brief GUI for slices interpolation.

  \ingroup Reliver
  \ingroup Widgets

  \sa QmitkInteractiveSegmentation
  \sa mitk::SegmentationInterpolation
  
  There is a separate page describing the general design of QmitkInteractiveSegmentation: \ref QmitkInteractiveSegmentationTechnicalPage

  While mitk::SegmentationInterpolation does the bookkeeping of interpolation 
  (keeping track of which slices contain how much segmentation) and the algorithmic work, 
  QmitkSlicesInterpolator is responsible to watch the GUI, to notice, which slice is currently
  visible. It triggers generation of interpolation suggestions and also triggers acception of
  suggestions.

  \todo show/hide feedback on demand

  Last contributor: $Author: maleike $
*/
class QMITKEXT_EXPORT QmitkSlicesInterpolator : public QWidget
{
  Q_OBJECT

  public:
    
    QmitkSlicesInterpolator(QWidget* parent = 0, const char* name = 0);

    /**
      To be called once before real use.
      */
    void Initialize(mitk::ToolManager* toolManager, QmitkStdMultiWidget* multiWidget);

    virtual ~QmitkSlicesInterpolator();

    void SetDataStorage( mitk::DataStorage& storage );
    mitk::DataStorage* GetDataStorage();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnToolManagerWorkingDataModified();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnToolManagerReferenceDataModified();

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnTransversalTimeChanged(itk::Object* sender, const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnSagittalTimeChanged(itk::Object* sender, const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnFrontalTimeChanged(itk::Object* sender, const itk::EventObject&);
 
    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnTransversalSliceChanged(const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnSagittalSliceChanged(const itk::EventObject&);

    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnFrontalSliceChanged(const itk::EventObject&);
    
    /**
      Just public because it is called by itk::Commands. You should not need to call this.
    */
    void OnInterpolationInfoChanged(const itk::EventObject&);

  signals:

  public slots:

    /**
      Call this from the outside to enable/disable interpolation
    */
   void EnableInterpolation(bool);

    /**
      Call this from the outside to accept all interpolations
    */
   void FinishInterpolation(int windowID = -1);

  protected slots:

    /**
      Reaction to button clicks.
    */
    void OnAcceptInterpolationClicked();

    /*
      Opens popup to ask about which orientation should be interpolated
    */
    void OnAcceptAllInterpolationsClicked();

    /*
     * Will trigger interpolation for all slices in given orientation (called from popup menu of OnAcceptAllInterpolationsClicked)
     */
    void OnAcceptAllPopupActivated(QAction* action);
    
    /**
      Called on activation/deactivation
    */
    void OnInterpolationActivated(bool);

    void OnMultiWidgetDeleted(QObject*);

  protected:

    const std::map<QAction*, unsigned int> createActionToSliceDimension();
    const std::map<QAction*, unsigned int> ACTION_TO_SLICEDIMENSION;

    void AcceptAllInterpolations(unsigned int windowID);
        
    /**
      Retrieves the currently selected PlaneGeometry from a SlicedGeometry3D that is generated by a SliceNavigationController
      and calls Interpolate to further process this PlaneGeometry into an interpolation.

      \param e is a actually a mitk::SliceNavigationController::GeometrySliceEvent, sent by a SliceNavigationController
      \param windowID is 2 for transversal, 1 for frontal, 0 for sagittal (similar to sliceDimension in other methods)
          */
    bool TranslateAndInterpolateChangedSlice(const itk::EventObject& e, unsigned int windowID);
    
    /**
      Given a PlaneGeometry, this method figures out which slice of the first working image (of the associated ToolManager)
      should be interpolated. The actual work is then done by our SegmentationInterpolation object.
     */
    void Interpolate( mitk::PlaneGeometry* plane, unsigned int timeStep );
    
    /**
      Called internally to update the interpolation suggestion. Finds out about the focused render window and requests an interpolation.
     */
    void UpdateVisibleSuggestion();
    
    /**
     * Tries to figure out the slice position and orientation for a given render window.
     * \param windowID is 2 for transversal, 1 for frontal, 0 for sagittal (similar to sliceDimension in other methods)
     * \return false if orientation could not be determined
     */
    bool GetSliceForWindowsID(unsigned windowID, int& sliceDimension, int& sliceIndex);

    mitk::SegmentationInterpolationController::Pointer m_Interpolator;

    QmitkStdMultiWidget* m_MultiWidget;
    mitk::ToolManager* m_ToolManager;
    bool m_Initialized;

    unsigned int TSliceObserverTag;
    unsigned int SSliceObserverTag;
    unsigned int FSliceObserverTag;
    unsigned int TTimeObserverTag;
    unsigned int STimeObserverTag;
    unsigned int FTimeObserverTag;
    unsigned int InterpolationInfoChangedObserverTag;

    QPushButton* m_BtnAcceptInterpolation;
    QPushButton* m_BtnAcceptAllInterpolations;

    mitk::DataTreeNode::Pointer m_FeedbackNode;

    mitk::Image* m_Segmentation;
    unsigned int m_LastSliceDimension;
    unsigned int m_LastSliceIndex;
    
    std::vector<unsigned int> m_TimeStep; // current time step of the render windows

    bool m_InterpolationEnabled;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
};

#endif

