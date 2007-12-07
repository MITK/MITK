/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkToolWorkingDataSelectionListBox_h_Included
#define QmitkToolWorkingDataSelectionListBox_h_Included

#include <qlistview.h>

#include "mitkToolManager.h"

class QCustomEvent;

/**
  \brief Display the data selection of a ToolManager.

  \sa mitk::ToolManager
  \sa mitk::DataStorage

  \ingroup Reliver
  \ingroup Widgets

  There is a separate page describing the general design of QmitkSliceBasedSegmentation: \ref QmitkSliceBasedSegmentationTechnicalPage

  Shows the working data of a ToolManager in a segmentation setting. By default only the segmentation name is shown.
  Additional segmentation properties (like volume) might be shown - SetAdditionalColumns has to be called then.

  The working images (segmentations) are listed in a QListView, each row telling the color and name
  of a single segmentation. One or several segmentations can be selected to be the "active" segmentations.

  $Author$
*/
class QmitkToolWorkingDataListBox : public QListView
{
  Q_OBJECT

  public:
    
    QmitkToolWorkingDataListBox(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkToolWorkingDataListBox();

    /**
      Installs an event filter on the given object. QmitkToolWorkingDataListBox will catch all key-press events
      for the number keys 0 to 9 and select the appropriate segmentation, when a segmentation's index is matched.

      \warning If you call this function, don't forget to deactivate event filtering (e.g. when your functionality is deactivated)
      by calling again with parameter NULL.
    */
    void InstallKeyFilterOn( QObject* object );
    
    /**
      \warning Does not do anything yet.

      When called with true, the list widget will take care that only the selected segmentation is visible and all other segmentations invisible.
      When called with false, the opposite behaviour is the case - the widget will not care for the visibilities - it will only once switch all segmentations
      to visible when this method is called.
      */
    void SetShowOnlySelected(bool);

    void UpdateDataDisplay();
    
    mitk::ToolManager* GetToolManager();
    void SetToolManager(mitk::ToolManager&); // no NULL pointer allowed here, a manager is required

    mitk::ToolManager::DataVectorType GetAllNodes( bool onlyDerivedFromOriginal = true );
    mitk::ToolManager::DataVectorType GetSelectedNodes();
    mitk::DataTreeNode* GetSelectedNode();
    
    void OnToolManagerWorkingDataModified(const itk::EventObject&);
    void OnToolManagerReferenceDataModified(const itk::EventObject&);

    // \param columns Comma separated list of key:title pairs please. E.g. 'SetAdditionalColumns( "name:Name,volume:Vol." )'
    void SetAdditionalColumns(const std::string& columns);

  signals:

    void WorkingNodeSelected(const mitk::DataTreeNode*);
    void DeleteKeyPressed();
    
  protected slots:
    
    void OnWorkingDataSelectionChanged();
    void itemRightClicked( QListViewItem*, const QPoint&, int );

  protected:

    typedef std::map< QListViewItem*, mitk::DataTreeNode* > ItemNodeMapType;
    typedef std::vector< std::pair<std::string, std::string> > ColumnsList;
    typedef std::vector<std::string> StringListType;

    virtual void customEvent( QCustomEvent* );
    void UpdateDataDisplayLater();
    StringListType Split( const std::string& wholeString, const char separator = ',', const char escape = '\\');
    
    void keyPressEvent( QKeyEvent * e );
    void keyReleaseEvent( QKeyEvent * e );

    /// used to filter shortcut keys (1,2,3...)
    bool eventFilter( QObject *o, QEvent *e );

    void UpdateNodeVisibility();

    mitk::ToolManager::Pointer m_ToolManager;

    ItemNodeMapType m_Node;
    ColumnsList m_AdditionalColumns;
    
    unsigned long m_ToolWorkingDataChangedObserverTag;
    unsigned long m_ToolReferenceDataChangedObserverTag;
    
    bool m_ShowOnlySelected;
    
    bool m_SelfCall;
    
    QObject* m_LastKeyFilterObject;

    mitk::DataTreeNode* m_LastSelectedReferenceData;
};
 
#endif

