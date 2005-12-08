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

QmitkListViewItemIndex::QmitkListViewItemIndex(QGridLayout* grid, QmitkListViewItemIndex* parentIndex)
: m_Grid(grid),
  m_Locked(0),
  m_ParentIndex(parentIndex)
{
  if (!grid) throw std::invalid_argument("NULL pointer for grid makes no sense in QmitkListViewExpanderIcon()");
}

// protected (for QmitkDataTreeListView
QmitkListViewItemIndex::QmitkListViewItemIndex()
: m_Grid(NULL),
  m_Locked(0),
  m_ParentIndex(NULL)
{
}

QmitkListViewItemIndex::~QmitkListViewItemIndex()
{
}

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

/// If some rows are missing, or if some rows do not hold both a QWidget and an Item,
/// then ONLY front-to-back build-order is supported, i.e. start with row 0 and then
/// go on with row 1, row 2, etc.
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

/// If some rows are missing, or if some rows do not hold both a QWidget and an Item,
/// then ONLY front-to-back build-order is supported, i.e. start with row 0 and then
/// go on with row 1, row 2, etc.
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

int QmitkListViewItemIndex::rowAt(int y) 
{
  // y coordinate -> row index
  if ( m_Grid->numRows() > 0)
    for (int row = 0; row < m_Grid->numRows(); ++row)
    {
      QRect cell( m_Grid->cellGeometry(row, m_Grid->numCols()-1) );
      if ( cell.top()-2 <= y && cell.bottom()+2 >= y ) // 2 was chosen because it's half the spacing of the gridlayouts 
        return row;                                   // which is set in QmitkDataTreeListView.cpp
    }
  
  return -1; // defaul = not found
}

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

/// throws std::out_of_range on bad row
std::list<QWidget*>& QmitkListViewItemIndex::widgetsAt(int row) 
{
  return m_Rows.at(row).second;
}

void QmitkListViewItemIndex::lockBecauseOfSelection(bool locked)
{
  if (locked)
    ++m_Locked;
  else
    --m_Locked;
}

QmitkListViewItemIndex* QmitkListViewItemIndex::parentIndex() 
{
  return m_ParentIndex;
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
  
void QmitkListViewExpanderIcon::setExpanded(bool expanded)
{
  m_Expanded = expanded;

  displayCorrectIcon();
    
  setAllChildrenVisible(m_Expanded);
}

void QmitkListViewExpanderIcon::mouseReleaseEvent ( QMouseEvent * e )
{
  if (!m_Locked)
    setExpanded(!m_Expanded); //toggle status
  else
  {
    // bing
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

