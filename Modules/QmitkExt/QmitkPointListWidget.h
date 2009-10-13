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

#ifndef QmitkPointListWidgetHIncluded
#define QmitkPointListWidgetHIncluded

#include "mitkDataTreeNode.h"
#include "mitkPointSetInteractor.h"

#include "QmitkPointListView.h"

class QPushButton;

/*!
 * \brief Widget for regular operations on point sets
 *
 * Displays a list of point coordinates and a couple of
 * buttons which
 *
 * \li enable point set interaction
 * \li clear all points from a set
 * \li load points from file
 * \li save points to file
 *
 * The user/application module of this widget needs to
 * assign a mitk::PointSet object to this widget. The user
 * also has to decide whether it wants to put the point set
 * into (a) DataStorage. This widget will not add/remove
 * point sets to DataStorage.
 *
 * If the render window crosshair should be moved to the
 * currently selected point, the widget user has to provide
 * a QmitkStdMultiWidget object.
 */
class QMITKEXT_EXPORT QmitkPointListWidget : public QWidget
{
  Q_OBJECT

  public:

    QmitkPointListWidget( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QmitkPointListWidget();

    /// assign a point set (contained in a node of DataStorage) for observation
    void SetPointSetNode( mitk::DataTreeNode* node );
    
    /// assign a QmitkStdMultiWidget for updating render window crosshair
    void SetMultiWidget( QmitkStdMultiWidget* multiWidget );
    
    /// itk observer for node "delete" events
    void OnNodeDeleted( const itk::EventObject & e );

  signals:

    /// signal to inform about cleared or loaded point sets
    void PointListChanged();

    /// signal to inform about the state of the EditPointSetButton, whether an interactor for setting points is active or not
    void EditPointSets(bool active);

    /// signal to inform that the selection of a point in the pointset has changed
    void PointSelectionChanged(); 
  public slots:

    void DeactivateInteractor(bool deactivate);
    void EnableEditButton(bool enabled);
 
  protected slots:

    void OnEditPointSetButtonToggled(bool checked);
    void OnClearPointSetButtonClicked();
    void OnLoadPointSetButtonClicked();
    void OnSavePointSetButtonClicked();

  protected:

    void ObserveNewNode( mitk::DataTreeNode* node );

    QmitkPointListView* m_ListView;
    QPushButton* m_BtnEdit;
    QPushButton* m_BtnClear;
    QPushButton* m_BtnLoad;
    QPushButton* m_BtnSave;
    mitk::DataTreeNode* m_PointSetNode;
    mitk::PointSetInteractor::Pointer m_Interactor;
    bool m_EditAllowed;
    unsigned int m_NodeObserverTag;
};

#endif

