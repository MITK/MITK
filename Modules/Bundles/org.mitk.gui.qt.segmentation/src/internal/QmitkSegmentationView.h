/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#include "mitkDataTreeNodeSelection.h"
#include "mitkSegmentationSelectionProvider.h"

#include "QmitkFunctionality.h"

#include <berryIBerryPreferences.h>

#include "ui_QmitkSegmentationControls.h"

/**
 * \ingroup org_mitk_gui_qt_segmentation_internal
 * \warning Implementation of this class is split up into two .cpp files to make things more compact. Check both this file and QmitkSegmentationOrganNamesHandling.cpp
 */
class QmitkSegmentationView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

  public:

    QmitkSegmentationView();
    virtual ~QmitkSegmentationView();
      
    // reaction to new segmentations being created by segmentation tools
    void NewNodesGenerated();
    void NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType*);

    // QmitkFunctionality's activated/deactivated status changes
    virtual void Activated();
    virtual void Deactivated();

    // QmitkFunctionality's changes regarding THE QmitkStdMultiWidget
    virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();
    virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);
  
    // BlueBerry's notification about preference changes (e.g. from a dialog)
    void PreferencesChanged(const berry::IBerryPreferences*);

  protected slots:

    // reaction to the button "New segmentation"
    void CreateNewSegmentation();

    // called when a segmentation tool is activated
    void ManualToolSelected(int id);
    
    // called when one of "Manual", "Organ", "Lesion" pages of the QToolbox is selected
    void ToolboxStackPageChanged(int id);

  protected:
    
    // a type for handling lists of DataTreeNodes
    typedef std::vector<mitk::DataTreeNode*> NodeList;

    // actively query the current selection of data manager
    void PullCurrentDataManagerSelection();

    // reactions to selection events from data manager (and potential other senders)
    void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);
    void ReferenceNodeSelected(const mitk::DataTreeNode*);
    void WorkingDataSelectionChanged(const mitk::DataTreeNode*);
    
    // sending of selection events to data manager (an potential other observers)
    void SendSelectedEvent( mitk::DataTreeNode* referenceNode, mitk::DataTreeNode* workingNode );

    // get the current selection of data manager (or other selection senders)
    NodeList GetSelectedNodes() const;
   
    // checks if selected reference image is aligned with the slices stack orientation of the StdMultiWidget
    void CheckImageAlignment();

    // GUI setup
    void CreateQtPartControl(QWidget* parent);

    // decorates a DataTreeNode according to the user preference settings
    void ApplyDisplayOptions(mitk::DataTreeNode* node);
    
    // handling of a list of known (organ name, organ color) combination
    // ATTENTION these methods are defined in QmitkSegmentationOrganNamesHandling.cpp
    QStringList GetDefaultOrganColorString();
    void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname);
    void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b);
  
    // the Qt parent of our GUI (NOT of this object)
    QWidget* m_Parent;

    // our GUI
    Ui::QmitkSegmentationControls * m_Controls;

    // THE currently existing QmitkStdMultiWidget
    QmitkStdMultiWidget * m_MultiWidget;
 
    // buffer for the data manager selection
    mitk::DataTreeNodeSelection::ConstPointer m_CurrentSelection;

    // object to observe BlueBerry selections
    berry::ISelectionListener::Pointer m_SelectionListener;
    // helper stuff to observe BlueBerry selections
    friend struct berry::SelectionChangedAdapter<QmitkSegmentationView>;

    // helper class to create/send BlueBerry selection events
    mitk::SegmentationSelectionProvider::Pointer m_SelectionProvider;
  
    // container of this view's user preferences (automatically retrieved/stored by BlueBerry)
    berry::IBerryPreferences::Pointer m_SegmentationPreferencesNode;
};

#endif /*QMITKsegmentationVIEW_H_*/

