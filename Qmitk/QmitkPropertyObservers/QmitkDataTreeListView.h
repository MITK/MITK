#ifndef QMITK_DATATREELISTVIEW_H_INCLUDED_SNewS
#define QMITK_DATATREELISTVIEW_H_INCLUDED_SNewS

/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <qlistview.h>
#include <QmitkDataTreeListViewExpander.h>

/**
 @brief Displays hierarchical items of a DataTreeFilter
 @ingroup Widgets
 
 \section sectionQmitkDataTreeListViewOverview Overview
 
 This class provides a graphical view of a DataTreeFilter. It displays all the contained items, including
 the hierarchy and allows for multi-selections.

 \section sectionQmitkDataTreeListViewUsage Usage

 There are two principal ways to initialize this class. Either you create a
 DataTreeFilter and then tell the QmitkDataTreeListView about it, or you let the
 view itself create a tree filter by giving it a pointer to the data tree. You
 can always access (and modify) the underlying DataTreeFilter through
 GetFilter().

 \subsection sectionQmitkDataTreeListViewInit1 Initialization 1: using a ready-made DataTreeFilter

 \e You own both the tree filter and the view widget:
\code
  mitk::DataTreeFilter* treeFilter = bar();  // see documentation of DataTreeFilter 
                                            //  on how to configure this class

  if ( ) // you are just creating your widgets
  {
    QmitkDataTreeListView* listView = new QmitkDataTreeListView(treeFilter, this); // create a new Qt widget
  }
  else   // widgets already exists
  {
    listView->SetFilter( treeFilter );
  }
\endcode
 
 \subsection sectionQmitkDataTreeListViewInit2 Initialization 2: implicit creation of a DataTreeFilter

 You own the view widget, the view widget own the tree filter:
\code
  if ( ) // you are just creating your widgets
  {
    QmitkDataTreeListView* listView = new QmitkDataTreeListView( GetDataTree(), this); // create a new Qt widget
  }
  else   // widgets already exists
  {
    listView->SetDataTree( GetDataTree() );
  }
\endcode
 
 \subsection sectionQmitkDataTreeListViewInit3 Changing the "stretched" column

 Oftenly there will be more space available for the widget than is actually
 needed to display all items. When this happens, Qt enlarges with width of one of the
 displayed columns to use more space, which is called stretching. By default, the right-most column is
 strechted. 
 In case you want to have another column stretched, use SetStretchedColumn() to do so.
 
\subsection sectionQmitkDataTreeListViewInit5 Signals and when they are emited

 - \e activated(item, bool) is emitted, whenever the selection of any item in the list changes. This can occur as
      a result of user interaction with this widget, of due to a change in the underlying DataTreeFilter or the DataTree itself.
      The bool parameter will tell you if the affected item is now selected or not.
 - \e clicked(item, bool) is only emitted, when the selection of any item in the list changes because it was clicked in this
      widget. The signal is not emitted when a selection changes due to changes in the DataTreeFilter.
      The bool parameter will tell you if the affected item is now selected or not.
  
 \todo{This class is still work in progress: if further signals are required by anyone, file bugs or mail your requirements.}
 
 
 \section sectionQmitkDataTreeListViewImplementation Implementation
 \subsection sectionQmitkDataTreeListViewImplementation1 Synchronization with the tree filter

 All communication from the DataTreeFilter to this view is done via itk::Events. I.e. the QmitkDataTreeListView
 installs a handful of listeners on a given DataTreeFilter and reacts to changes (see the ...Handler() methods).

 The only communication from this view to the DataTreeFilter is the selection state of single items. To change this,
 mitk::DataTreeFilter::Item::SetSelected() is called on the appropriate items.

 \subsection sectionQmitkDataTreeListViewImplementation2 Layout of child widgets

 QmitkDataTreeListView uses Qt derivations of mitk::PropertyView to display the
 properties of single items. These derived classes are all QWidgets, so they
 cannot be easily displayed in a QListView. Instead, this class does the layout
 of its child widgets on its own, using a hierarchy of QGridLayouts.

  To allow for maintenance of this class, the following paragraph sketches, how
  the layout is done:

  \image html doc_QmitkDataTreeListViewChildrenLayout.png Illustration of children layout
  \image latex doc_QmitkDataTreeListViewChildrenLayout.eps Illustration of children layout

  All top-level widgets are contained in one Gridlayout, one row per item.
  For each displayed property of each item, one view widget is created. These
  widgets are placed in the Gridlayout left-to-right, beginning with column 1(!).
  Column zero is reserved for an expand/collapse icon of those items, that have
  children.

  When an item has children, a QmitkListViewExpanderIcon is inserted in column 0.
  This expander icon owns and manages a newly created Gridlayout (green in the
  diagram above), where observer widgets for the child items' properties are
  placed. This placing of sub-GridLayouts is done recursively for all children
  lists.

*/
class QMITK_EXPORT QmitkDataTreeListView : public QWidget, public QmitkListViewItemIndex
{
  Q_OBJECT;

  public:

    typedef mitk::DataTreeFilter::PropertyList PropertyList;

    QmitkDataTreeListView(QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeFilter* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeBase* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeIteratorBase* filter,QWidget* parent = 0, const char* name = 0);
    ~QmitkDataTreeListView();

    virtual void SetDataTree(mitk::DataTreeBase*);
    virtual void SetDataTree(mitk::DataTreeIteratorBase*);

    virtual void SetFilter(mitk::DataTreeFilter*);
    virtual mitk::DataTreeFilter* GetFilter();

    virtual void SetViewType(const std::string& property, unsigned int type); /// use QmitkPropertyViewFactory::ViewTypes or QmitkPropertyViewFactory::EditorTypes for type

    virtual int stretchedColumn();
    virtual void setStretchedColumn(int);
   
    virtual QSize sizeHint() const;

    // handler for event notification
    virtual void removeItemHandler( const itk::EventObject& e );
    virtual void removeChildrenHandler( const itk::EventObject& e );
    virtual void removeAllHandler( const itk::EventObject& e );
    virtual void selectionChangedHandler( const itk::EventObject& e );
    virtual void itemAddedHandler( const itk::EventObject& e );
    virtual void updateAllHandler( const itk::EventObject& e );
    virtual void newItemHandler( const itk::EventObject& e );
    
    virtual void SetAutoUpdate(bool);
    virtual void Update();

  signals:
    
    virtual void clicked(const mitk::DataTreeFilter::Item*, bool selected);
    virtual void activated(const mitk::DataTreeFilter::Item*, bool);
    virtual void newItem(const mitk::DataTreeFilter::Item*);
   
  protected:

    virtual void initialize();
    virtual void generateItems();

    virtual void paintListBackground(QPainter& painter, QmitkListViewItemIndex* index);
    virtual void paintEvent(QPaintEvent*);

    virtual void mouseReleaseEvent (QMouseEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);

    virtual void clearItems();   

    typedef std::map<std::string, unsigned int> PropertyViewTypeMap;

    virtual void AddItemsToList(QWidget* parent, QmitkListViewItemIndex* index,
                                               const mitk::DataTreeFilter::ItemList* items,
                                               const mitk::DataTreeFilter::PropertyList& visibleProps,
                                               const mitk::DataTreeFilter::PropertyList& editableProps);

    virtual void connectNotifications();
    virtual void disconnectNotifications();

    virtual bool selectItemInGrid(const mitk::DataTreeFilter::Item* item, bool selected, QmitkListViewItemIndex* index);
    
    mitk::DataTreeFilter::Pointer m_DataTreeFilter;
    int m_StretchedColumn;

    QSize m_SizeHint;
    
    mitk::DataTreeFilter::Pointer m_PrivateFilter;
    const mitk::DataTreeFilter::Item* m_SkipItem;
    const mitk::DataTreeFilter::Item* m_SkipItemParent;

    unsigned long  m_RemoveItemConnection;
    unsigned long  m_RemoveChildrenConnection;
    unsigned long  m_RemoveAllConnection;
    unsigned long  m_SelectionChangedConnection;
    unsigned long  m_ItemAddedConnection;
    unsigned long  m_UpdateAllConnection;
    unsigned long  m_NewItemConnection;

    bool m_SelfCall;
    
    bool m_AutoUpdate;

    PropertyViewTypeMap m_ViewTypes;
};

#endif

