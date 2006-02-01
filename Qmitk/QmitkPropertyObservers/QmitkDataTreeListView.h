#ifndef QMITK_DATATREELISTVIEW_H_INCLUDED_SNewS
#define QMITK_DATATREELISTVIEW_H_INCLUDED_SNewS

/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
 @brief Displays items of a mitk::DataTreeFilter

 This class provides a view on a mitk::DataTreeFilter. It displays all the contained items, including
 the hierarchy and allows for multi-selections.

 All communication from the DataTreeFilter to this view is done via itk::Events. I.e. the QmitkDataTreeListView
 installs a handful of listeners on a given DataTreeFilter and reacts to changes.

 The only communication from this view to the DataTreeFilter is the selection state of single items. To change this,
 mitk::DataTreeFilter::Item::SetSelected() is called on the appropriate items.

 Initialization of a QmitkDataTreeView is done either through the constructor (if a data tree is available at the
 time you want to call the constructor), or through one of the methods SetDataTree(), SetFilter().
 
 TODO a source code example on how to use this class should be added
*/
class QmitkDataTreeListView : public QWidget, public QmitkListViewItemIndex
{
  Q_OBJECT;

  public:

    typedef mitk::DataTreeFilter::PropertyList PropertyList;

    QmitkDataTreeListView(QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeFilter* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeBase* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeIteratorBase* filter,QWidget* parent = 0, const char* name = 0);
    ~QmitkDataTreeListView();

    void SetDataTree(mitk::DataTreeBase*);
    void SetDataTree(mitk::DataTreeIteratorBase*);

    void SetFilter(mitk::DataTreeFilter*);
    mitk::DataTreeFilter* GetFilter();

    int stretchedColumn();
    void setStretchedColumn(int);
   
    virtual QSize sizeHint() const;

    // handler for event notification
    void removeItemHandler( const itk::EventObject& e );
    void removeChildrenHandler( const itk::EventObject& e );
    void removeAllHandler( const itk::EventObject& e );
    void selectionChangedHandler( const itk::EventObject& e );
    void itemAddedHandler( const itk::EventObject& e );
    void updateAllHandler( const itk::EventObject& e );
   
  protected:

    void initialize();
    void generateItems();

    void paintListBackground(QPainter& painter, QmitkListViewItemIndex* index);
    virtual void paintEvent(QPaintEvent*);

    virtual void mouseReleaseEvent (QMouseEvent*);

    void clearItems();
    
  private:

    void QmitkDataTreeListView::AddItemsToList(QWidget* parent, QmitkListViewItemIndex* index,
                                               const mitk::DataTreeFilter::ItemList* items,
                                               const mitk::DataTreeFilter::PropertyList& visibleProps,
                                               const mitk::DataTreeFilter::PropertyList editableProps);

    void connectNotifications();
    void disconnectNotifications();

    bool selectItemInGrid(const mitk::DataTreeFilter::Item* item, bool selected, QmitkListViewItemIndex* index);
    
    mitk::DataTreeFilter* m_DataTreeFilter;
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

    bool m_SelfCall;
};

#endif

