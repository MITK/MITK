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

#ifndef QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb
#define QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb

#include<qlabel.h>
#include<list>
#include<mitkDataTreeFilter.h>

class QGridLayout;

/**
  @brief Helper class for QmitkDataTreeListView (does the indexing of items).

  This class manages the information about what widgets are contained in it, 
  where on the screen (which row) to find them.

  A QmitkListViewItemIndex can have children, which results in a tree-like structure.
  When hierarchies are displayed, the QmitkDataTreeListView uses QmitkListViewExpanderIcon to
  show/hide subtrees on user request.

  On-screen arrangement of widget is done in a QGridLayout, which has most of the necessary methods.
  The row structuring is done in a vector of items and widgets. Each item in this vector represents one
  row on the screen. One row always corresponds to one mitk::DataTreeFilter::Item and a list of widgets
  (which display different properties of a mitk::DataTreeNode).
*/
class QMITK_EXPORT QmitkListViewItemIndex
{
  public:

    QmitkListViewItemIndex(QGridLayout* layout, QmitkListViewItemIndex* parentIndex);
    ~QmitkListViewItemIndex();

    void addIndex(QmitkListViewItemIndex*, int row);
    void addWidget(QWidget*, int row, int col, int alignment = 0);
    void addMultiCellWidget(QWidget*, int fromRow, int toRow, int fromCol, int toCol, int alignment = 0);
    void addItem(mitk::DataTreeFilter::Item*, int row);
    
    int rowAt(int y);  /// y coordinate -> row index

    QmitkListViewItemIndex* indexAt(int row);      /// index for sub-items in a row
    mitk::DataTreeFilter::Item* itemAt(int row);  /// mitkDataTreeFilter::Item of a row
    std::list<QWidget*>& widgetsAt(int row);     /// widgets of a row
    
    void lockBecauseOfSelection(bool);
    QmitkListViewItemIndex* parentIndex();

    QGridLayout* m_Grid;
    
  protected:

    typedef std::vector< std::pair< mitk::DataTreeFilter::Item*, std::list<QWidget*> > > RowStructureType;

    QmitkListViewItemIndex();
    void clearIndex();
    
    RowStructureType m_Rows;
    std::vector<QmitkListViewItemIndex*> m_Indices;

    int m_Locked;
    QmitkListViewItemIndex* m_ParentIndex;

  private:
};

/**
  @brief Helper class for QmitkDataTreeListView (Shows/hides subtrees).

  Not much new information here, if you know QmitkListViewItemIndex. This class only adds
  show/hide methods.
*/
class QMITK_EXPORT QmitkListViewExpanderIcon : public QLabel, public QmitkListViewItemIndex
{
  Q_OBJECT

  public:
    
    QmitkListViewExpanderIcon( QGridLayout* childContainer, QmitkListViewItemIndex* parentIndex, QWidget* parent, const char* name = 0 );
    virtual ~QmitkListViewExpanderIcon();

    bool expanded();
    void setExpanded(bool);

    virtual QSize sizeHint();
    
  protected:
   
    virtual void mouseReleaseEvent (QMouseEvent*);

    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);

  protected slots:

    void displayCorrectIcon();
    
  private:

    void setAllChildrenVisible(bool);

    bool m_Expanded;
};


#endif

