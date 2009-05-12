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

#include <QmitkDataTreeListViewExpander.h>

#include <qpixmap.h>
#include <qobjectlist.h>
#include <qlayout.h>
#include <qtimer.h>

#include <stdexcept>

#include "collapsed.xpm"
#include "expanded.xpm"
#include "notpossible.xpm"

// for debugging
#include <iostream>

//--- QmitkListViewItemIndex ------------------------------------------------

/**
  This constructor may be only called for children, so the parent QmitkListViewItemIndex and the managed
  GridLayout have to be given.
*/
QmitkListViewItemIndex::QmitkListViewItemIndex(QGridLayout* grid, QmitkListViewItemIndex* parentIndex)
: m_Grid(grid),
  m_Locked(0),
  m_ParentIndex(parentIndex)
{
  if (!grid) throw std::invalid_argument("NULL pointer for grid makes no sense in QmitkListViewItemIndex()");
}

/**
  Protected for QmitkDataTreeListView, which creates m_Grid on updates (before any calls to addWidget, etc.). 
*/
QmitkListViewItemIndex::QmitkListViewItemIndex()
: m_Grid(NULL),
  m_Locked(0),
  m_ParentIndex(NULL)
{
}

QmitkListViewItemIndex::~QmitkListViewItemIndex()
{
}

/**
  Tells the class, that row \a row has now a QmitkListViewItemIndex, because the row has sub-items
  which need to be managed.
*/
void QmitkListViewItemIndex::addIndex(QmitkListViewItemIndex* index, int row)
{
  try
  {
    m_Indices.at(row) = index;
  }
  catch (std::out_of_range)
  {
    int oldSize = m_Indices.size();
    m_Indices.resize(row+1); // we need more elements
    m_Indices[row] = index;

    // Rows missing? Then fill with NULLs
    if ( m_Indices.size() - oldSize > 1 )
      for (int i = oldSize; i < row; ++i)
        m_Indices[i] = NULL;
  }
}

/**
  Adds a widget to the GridLayout and stores information about the widget.

  \warning Always add widgets top-to-bottom and left-to-right.
*/
void QmitkListViewItemIndex::addWidget(QWidget* widget, int row, int col, int alignment)
{ 
  if (widget)
  {
    if (m_Grid)
      m_Grid->addWidget(widget, row, col, alignment);

    try
    {
      m_Rows.at(row).second.push_back(widget);
    }
    catch (std::out_of_range)
    {
      m_Rows.resize(row+1); // we need more elements
      m_Rows[row].first = NULL;
      m_Rows[row].second.push_back(widget);
    }
  }
}
    
/**
  Adds a widget to the GridLayout and stores information about the widget.

  \warning Always add widgets top-to-bottom and left-to-right.
*/
void QmitkListViewItemIndex::addMultiCellWidget(QWidget* widget, int fromRow, int toRow, int fromCol, int toCol, int alignment)
{
  if (widget)
  {
    if (m_Grid)
      m_Grid->addMultiCellWidget(widget, fromRow, toRow, fromCol, toCol, alignment);

    try
    {
      m_Rows.at(fromRow).second.push_back(widget);
    }
    catch (std::out_of_range)
    {
      m_Rows.resize(toRow+1); // we need more elements
      m_Rows[fromRow].second.push_back(widget);
      for (int curRow = fromRow; curRow <= toRow; ++curRow)
      {
        m_Rows[curRow].first = NULL;
      }
    }
  }
}

/**
  Tell about the item that is associated to a row.

  \warning Always add items top-to-bottom and left-to-right.
*/
void QmitkListViewItemIndex::addItem(mitk::DataTreeFilter::Item* item, int row)
{
  if (item)
  {
    try
    {
      m_Rows.at(row).first = item;
    }
    catch (std::out_of_range)
    {
      m_Rows.resize(row+1); // we need more elements
      m_Rows[row].first = item;
    }
  }
}

/**
  \return Index of the row that contains the coordinate \a y, starting with row 0.
*/
int QmitkListViewItemIndex::rowAt(int y) 
{
  // y coordinate -> row index
  int r = -1;
  if ( m_Grid && m_Grid->numRows() > 0)
    for (int row = 0; row < m_Grid->numRows(); ++row)
    {
      QRect cell( m_Grid->cellGeometry(row, m_Grid->numCols()-1) );
      if ( cell.top()-2 <= y && cell.bottom()+2 >= y ) // 2 was chosen because it's half the spacing of the gridlayouts 
      {                                               // which is set in QmitkDataTreeListView.cpp
        r = row;
        break;
      }
    }
 
  // three lines fixing: makes selection of invisible rows impossible. 
  // This problem would better be fixed by not even showing invisible rows,
  // but currently they still show as 4 pixel empty lines because of setSpacing(4)...
  QmitkListViewExpanderIcon* i = static_cast<QmitkListViewExpanderIcon*>(indexAt(r-1));
  if ( i && !(i->expanded()) )
    --r;
  
  return r; // default = not found
}

/**
  \return QmitkListViewItemIndex that is associated with a row, starting with row 0.
*/
QmitkListViewItemIndex* QmitkListViewItemIndex::indexAt(int row) 
{
  try
  {
    return m_Indices.at(row);
  }
  catch (std::out_of_range)
  {
    return 0;
  }
}

/**
  \return mitk::DataTreeFilter::Item that is associated with a row, starting with row 0.
*/
mitk::DataTreeFilter::Item* QmitkListViewItemIndex::itemAt(int row) 
{
  try
  {
    return m_Rows.at(row).first;
  }
  catch (std::out_of_range)
  {
    return 0;
  }
}

/**
  \return A list of Qt widgets that is associated with a row, starting with row 0.
 
  Throws std::out_of_range on bad row
*/
std::list<QWidget*>& QmitkListViewItemIndex::widgetsAt(int row) 
{
  return m_Rows.at(row).second;
}

/**
  A basic locking mechanism is implemented to prevent the user from collapsing subtrees that contain selected
  items (thus hiding and removing from his attention a selection).
  
  Putting this here instead of QmitkListViewExpanderIcon is lazy, because QmitkDataTreeListView never needs locking,
  but it makes the handling easier.
*/
void QmitkListViewItemIndex::lockBecauseOfSelection(bool locked)
{
  if (locked)
    ++m_Locked;
  else
    --m_Locked;
}

/**
  \return The parent index.
*/
QmitkListViewItemIndex* QmitkListViewItemIndex::parentIndex() 
{
  return m_ParentIndex;
}

/**
  Clears all managed information. This is needed for rebuilding QmitkDataTreeListView completely.
*/
void QmitkListViewItemIndex::clearIndex() 
{
  m_Rows.clear();
  m_Indices.clear();
  m_Locked = 0;
}

//--- QmitkListViewExpanderIcon ---------------------------------------------
QmitkListViewExpanderIcon::QmitkListViewExpanderIcon( QGridLayout* grid, QmitkListViewItemIndex* parentIndex, QWidget* parent, const char* name )
: QLabel(parent, name),
  QmitkListViewItemIndex(grid, parentIndex),
  m_Expanded(true)
{
  setExpanded(m_Expanded);
}

QmitkListViewExpanderIcon::~QmitkListViewExpanderIcon()
{
}

bool QmitkListViewExpanderIcon::expanded()
{
  return m_Expanded;
}
 
QSize QmitkListViewExpanderIcon::sizeHint()
{
  return QSize(16, 16);
}

void QmitkListViewExpanderIcon::setExpanded(bool expanded)
{
  m_Expanded = expanded;

  displayCorrectIcon();
    
  setAllChildrenVisible(m_Expanded);

  if (parent()) 
    static_cast<QWidget*>(parent())->update(); // redraw
}

void QmitkListViewExpanderIcon::mouseReleaseEvent ( QMouseEvent* )
{
  if (!m_Locked)
    setExpanded(!m_Expanded); //toggle status
  else
  {
    setPixmap( QPixmap(notpossible_xpm) );
    QTimer::singleShot(100,this,SLOT(displayCorrectIcon()));
  }
}

void QmitkListViewExpanderIcon::showEvent(QShowEvent* e)
{
  if (m_Expanded)
    setAllChildrenVisible(true);
  QLabel::showEvent(e);
}

void QmitkListViewExpanderIcon::hideEvent(QHideEvent* e)
{
  if (m_Expanded)
    setAllChildrenVisible(false);
  QLabel::hideEvent(e);
}

void QmitkListViewExpanderIcon::displayCorrectIcon()
{
  if (m_Expanded)
    setPixmap( QPixmap(expanded_xpm) );
  else
    setPixmap( QPixmap(collapsed_xpm) );
}

void QmitkListViewExpanderIcon::setAllChildrenVisible(bool visible)
{
  // show/hide all children
  for (RowStructureType::iterator iterrow = m_Rows.begin();
       iterrow != m_Rows.end();
       ++iterrow)
    for (std::list<QWidget*>::iterator iterwidget = iterrow->second.begin();
        iterwidget != iterrow->second.end();
        ++iterwidget)
    {
      if (visible)
        (*iterwidget)->show();
      else
        (*iterwidget)->hide();
    }
}

