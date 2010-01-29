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

#ifndef QMITKsegmentationVIEW_H_
#define QMITKsegmentationVIEW_H_

#include <berryIPartListener.h>

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>

#include "ui_QmitkSegmentationControls.h"
#include <berryISelectionListener.h>
#include "mitkDataTreeNodeSelection.h"
#include <berryIBerryPreferences.h>

#include "mitkSegmentationSelectionProvider.h"

#include "QmitkSegmentationPostProcessing.h"

/**
 * \ingroup org_mitk_gui_qt_segmentation_internal
 */
class QmitkSegmentationView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

public:
  QStringList GetDefaultOrganColorString();

  QmitkSegmentationView();
  virtual ~QmitkSegmentationView();
    
  void OnNewNodesGenerated();
  void OnNewNodeObjectsGenerated(mitk::ToolManager::DataVectorType*);

  virtual void Activated();
  virtual void Deactivated();


  public:

    ///
    /// Called when a StdMultiWidget is available.
    ///
    virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
    ///
    /// Called when no StdMultiWidget is available.
    ///
    virtual void StdMultiWidgetNotAvailable();

    ///
    /// Called when no StdMultiWidget is getting closed.
    ///
    virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);
  
    void OnPreferencesChanged(const berry::IBerryPreferences*);
protected:

  void CreateQtPartControl(QWidget* parent);
  protected slots:

    void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);

    void CreateNewSegmentation();

    void OnToolSelected(int id);
    void OnSegmentationMethodSelected(int id);
    void OnReferenceNodeSelected(const mitk::DataTreeNode*);
    void OnWorkingDataSelectionChanged(const mitk::DataTreeNode*);

    void SendSelectedEvent( mitk::DataTreeNode* referenceNode, mitk::DataTreeNode* workingNode );

  protected:

    typedef std::vector<mitk::DataTreeNode*> NodeList;
    NodeList GetSelectedNodes() const;

    void CheckImageAlignment();
    void ApplyDisplayOptions(mitk::DataTreeNode* node);

    void UpdateFromCurrentDataManagerSelection();

    void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname);
    void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b);
  
    berry::IBerryPreferences::Pointer m_SegmentationPreferencesNode;

    QmitkStdMultiWidget * m_MultiWidget;

    QWidget* m_Parent;

    Ui::QmitkSegmentationControls * m_Controls;
    unsigned long m_ObserverTag;
    mitk::DataStorage::Pointer m_DataStorage;

    mitk::DataTreeNodeSelection::ConstPointer m_CurrentSelection;
    berry::ISelectionListener::Pointer m_SelectionListener;
    friend struct berry::SelectionChangedAdapter<QmitkSegmentationView>;
  
    bool m_ShowSegmentationsAsOutline;
    bool m_ShowSegmentationsAsVolumeRendering;

    mitk::SegmentationSelectionProvider::Pointer m_SelectionProvider;

    QStringList m_OrganColor;

    QmitkSegmentationPostProcessing* m_PostProcessing;

  /// from QmitkSegmentation

};

#endif /*QMITKsegmentationVIEW_H_*/
