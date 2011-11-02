/*======================================================================
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkSegmentationView_h
#define QmitkSegmentationView_h

#include "QmitkFunctionality.h"

#include <berryIBerryPreferences.h>

#include "ui_QmitkSegmentationControls.h"

class QmitkRenderWindow;
// class QmitkSegmentationPostProcessing;

/**
 * \ingroup ToolManagerEtAl
 * \ingroup org_mitk_gui_qt_segmentation_internal
 * \warning Implementation of this class is split up into two .cpp files to make things more compact. Check both this file and QmitkSegmentationOrganNamesHandling.cpp
 */
class QmitkSegmentationView : public QmitkFunctionality
{
  Q_OBJECT

public:

  QmitkSegmentationView();
  QmitkSegmentationView(const QmitkSegmentationView& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  virtual ~QmitkSegmentationView();

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;

  /*!
    \brief Invoked when the DataManager selection changed
    */
  virtual void OnSelectionChanged(mitk::DataNode* node);
  virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

  // reaction to new segmentations being created by segmentation tools
  void NewNodesGenerated();
  void NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType*);

  // QmitkFunctionality's activate/deactivate
  virtual void Activated();
  virtual void Deactivated();

  // QmitkFunctionality's changes regarding THE QmitkStdMultiWidget
  virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();
  virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);

  // BlueBerry's notification about preference changes (e.g. from a dialog)
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*);

  // observer to mitk::RenderingManager's RenderingManagerViewsInitializedEvent event
  void RenderingManagerReinitialized(const itk::EventObject&);

  // observer to mitk::SliceController's SliceRotation event
  void SliceRotation(const itk::EventObject&);

  static const std::string VIEW_ID;

protected slots:

  void OnComboBoxSelectionChanged(const mitk::DataNode* node);

  // reaction to the button "New segmentation"
  void CreateNewSegmentation();

  // reaction to the button "New segmentation"
  void CreateSegmentationFromSurface();

  // called when a segmentation tool is activated
  void ManualToolSelected(int id);

  // called when one of "Manual", "Organ", "Lesion" pages of the QToolbox is selected
  void ToolboxStackPageChanged(int id);

  void OnSurfaceSelectionChanged();

  //called when the checkbox Remember Contour Positions is selected/deselected
  void CheckboxRememberContourPositionsStateChanged(int state);

  void OnWorkingNodeVisibilityChanged();

protected:

  // a type for handling lists of DataNodes
  typedef std::vector<mitk::DataNode*> NodeList;

  // set available multiwidget
  void SetMultiWidget(QmitkStdMultiWidget* multiWidget);

  // actively query the current selection of data manager
  //void PullCurrentDataManagerSelection();

  // reactions to selection events from data manager (and potential other senders)
  //void BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);
  mitk::DataNode::Pointer FindFirstRegularImage( std::vector<mitk::DataNode*> nodes );
  mitk::DataNode::Pointer FindFirstSegmentation( std::vector<mitk::DataNode*> nodes );

  // propagate BlueBerry selection to ToolManager for manual segmentation
  void SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData);

  // checks if selected reference image is aligned with the slices stack orientation of the StdMultiWidget
  void CheckImageAlignment();

  // checks if given render window aligns with the slices of given image
  bool IsRenderWindowAligned(QmitkRenderWindow* renderWindow, mitk::Image* image);

  // make sure all images/segmentations look as selected by the users in this view's preferences
  void ForceDisplayPreferencesUponAllImages();

  // decorates a DataNode according to the user preference settings
  void ApplyDisplayOptions(mitk::DataNode* node);

  // GUI setup
  void CreateQtPartControl(QWidget* parent);

  // handling of a list of known (organ name, organ color) combination
  // ATTENTION these methods are defined in QmitkSegmentationOrganNamesHandling.cpp
  QStringList GetDefaultOrganColorString();
  void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname);
  void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b);

  // If a contourmarker is selected, the plane in the related widget will be reoriented according to the marker`s geometry
  void OnContourMarkerSelected (const mitk::DataNode* node);

  void NodeRemoved(const mitk::DataNode* node);

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

  // our GUI
  Ui::QmitkSegmentationControls * m_Controls;

  // THE currently existing QmitkStdMultiWidget
  QmitkStdMultiWidget * m_MultiWidget;

  // QmitkSegmentationPostProcessing* m_PostProcessing;

  unsigned long m_RenderingManagerObserverTag;
  unsigned long m_SlicesRotationObserverTag1;
  unsigned long m_SlicesRotationObserverTag2;
  unsigned long m_VisibilityChangedObserverTag;

  NodeTagMapType  m_WorkingDataObserverTags;

};

#endif /*QMITKsegmentationVIEW_H_*/

