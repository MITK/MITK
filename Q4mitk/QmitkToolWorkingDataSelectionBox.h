/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-14 14:16:02 +0200 (So, 14 Sep 2008) $
Version:   $Revision: 15245 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkToolWorkingDataSelectionListBox_h_Included
#define QmitkToolWorkingDataSelectionListBox_h_Included

// mmueller
#include <Q3ListView>

#include "mitkToolManager.h"
#include "mitkProperties.h"

class QCustomEvent;

/**
  \brief Display the data selection of a ToolManager.

  \sa mitk::ToolManager
  \sa mitk::DataStorage

  \ingroup Reliver
  \ingroup Widgets

  There is a separate page describing the general design of QmitkInteractiveSegmentation: \ref QmitkInteractiveSegmentationTechnicalPage

  Shows the working data of a ToolManager in a segmentation setting. By default only the segmentation name is shown.
  Additional segmentation properties (like volume) might be shown - SetAdditionalColumns has to be called then.

  The working images (segmentations) are listed in a QListView, each row telling the color and name
  of a single segmentation. One or several segmentations can be selected to be the "active" segmentations.

  $Author: maleike $
*/
class QMITK_EXPORT QmitkToolWorkingDataSelectionBox : public Q3ListView
{
  Q_OBJECT

  public:

    /**
     * \brief What kind of items should be displayed.
     *
     * Every mitk::Tool holds a NodePredicateBase object, telling the kind of data that this
     * tool will successfully work with. There are two ways that this list box deals with
     * these predicates.
     *
     *   DEFAULT is: list data if ANY one of the displayed tools' predicate matches.
     * Other option: list data if ALL one of the displayed tools' predicate matches
     */
    enum DisplayMode { ListDataIfAllToolsMatch, ListDataIfAnyToolMatches};
    
    QmitkToolWorkingDataSelectionBox(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkToolWorkingDataSelectionBox();

    /**
      Installs an event filter on the given object. QmitkToolWorkingDataSelectionBox will catch all key-press events
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

    /**
      \brief Can be called to trigger an update of the list contents.
     */
    void UpdateDataDisplay();
    
    /**
      \brief Returns the associated mitk::ToolManager.
      */
    mitk::ToolManager* GetToolManager();

    /**
      \brief Tell this object to listen to another ToolManager.
      */
    void SetToolManager(mitk::ToolManager&); // no NULL pointer allowed here, a manager is required

    /**
     * \brief A list of all displayed DataTreeNode objects.
     * This method might be convenient for program modules that want to display
     * additional information about these nodes, like a total volume of all segmentations, etc.
     */
    mitk::ToolManager::DataVectorType GetAllNodes( bool onlyDerivedFromOriginal = true );

    /**
     * \brief A list of all selected DataTreeNode objects.
     * This method might be convenient for program modules that want to display
     * additional information about these nodes, like a total volume of all segmentations, etc.
     */
    mitk::ToolManager::DataVectorType GetSelectedNodes();

    /**
     * \brief Like GetSelectedNodes(), but will only return one object.
     * Will only return what QListView gives as selected object (documentation says nothing is returned if list is in Single selection mode).
     */
    mitk::DataTreeNode* GetSelectedNode();
    
    /**
     * \brief Callback function, no need to call it.
     * This is used to observe and react to changes in the mitk::ToolManager object.
     */
    void OnToolManagerWorkingDataModified();
    
    /**
     * \brief Callback function, no need to call it.
     * This is used to observe and react to changes in the mitk::ToolManager object.
     */
    void OnToolManagerReferenceDataModified();

    /**
     * \brief Enhance the list with additional columns.
     *
     * Currently only used for volume display. Might not be suited for other properties.
     *
     * \param columns Comma separated list of key:title pairs please. E.g. 'SetAdditionalColumns( "name:Name,volume:Vol." )'
     */
    void SetAdditionalColumns(const std::string& columns);

    /**
     * \brief Set the title of the first column (default "Segmentation")
     */
    void SetFirstColumnTitle(const QString& title);

    /**
     * \brief How the list contents is determined.
     * 
     * See also documentation of DisplayMode.
     *
     * \sa DisplayMode
     */
    void SetDisplayMode( DisplayMode mode );

    /**
     * \brief No brief description.
     *
     * Should be called to restrict the number of tools that are
     * evaluated to build up the list. Default is to ask all tools for their predicate, by
     * setting the 'groups' string this can be restricted to certain groups of tools
     * or single tools.
     */
    void SetToolGroupsForFiltering(const std::string& groups);

    /**
     * \brief Wheter to show all segmentations or just those derived from reference data.
     */
    void SetDisplayOnlyDerivedNodes(bool on);

  signals:

    void WorkingNodeSelected(const mitk::DataTreeNode*);
    void DeleteKeyPressed();
    
  protected slots:
    
    void OnWorkingDataSelectionChanged();

    //!mm
    //void itemRightClicked( QListViewItem*, const QPoint&, int );
    void itemRightClicked( Q3ListViewItem*, const QPoint&, int );
    //!

  protected:

    typedef std::map< Q3ListViewItem*, mitk::DataTreeNode* > ItemNodeMapType;
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
    
    bool m_ShowOnlySelected;
    
    bool m_SelfCall;
    
    QObject* m_LastKeyFilterObject;

    mitk::DataTreeNode* m_LastSelectedReferenceData;

    DisplayMode m_DisplayMode;
    std::string m_ToolGroupsForFiltering;

    bool m_DisplayOnlyDerivedNodes;
};
 
#endif

