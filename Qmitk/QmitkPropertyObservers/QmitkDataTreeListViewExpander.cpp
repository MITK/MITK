#include <QmitkDataTreeListViewExpander.h>

#include <qpixmap.h>
#include <qobjectlist.h>
#include <qlayout.h>

#include <stdexcept>

#include "collapsed.xpm"
#include "expanded.xpm"

// for debugging
#include <iostream>

//--- QmitkListViewItemIndex ------------------------------------------------

QmitkListViewItemIndex::QmitkListViewItemIndex(QGridLayout* grid)
: m_Grid(grid)
{
  if (!grid) throw std::invalid_argument("NULL pointer for grid makes no sense in QmitkListViewExpanderIcon()");
}

// protected (for QmitkDataTreeListView
QmitkListViewItemIndex::QmitkListViewItemIndex()
: m_Grid(0)
{
}

QmitkListViewItemIndex::~QmitkListViewItemIndex()
{
}

void QmitkListViewItemIndex::addWidget(QWidget* widget)
{
  if (widget)
    m_Children.push_back(widget);
}

int QmitkListViewItemIndex::rowAt(int y)
{
  // y coordinate -> row index
  return -1;
}

QmitkListViewItemIndex* QmitkListViewItemIndex::childrenIndexAt(int row)
{
  return 0;
}

mitk::DataTreeFilter::Item* QmitkListViewItemIndex::itemAt(int row)
{
  return 0;
}

//--- QmitkListViewExpanderIcon ---------------------------------------------
QmitkListViewExpanderIcon::QmitkListViewExpanderIcon( QGridLayout* grid, QWidget* parent, const char* name )
: QLabel(parent, name),
  QmitkListViewItemIndex(grid),
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

  if (m_Expanded)
    setPixmap( QPixmap(expanded_xpm) );
  else
    setPixmap( QPixmap(collapsed_xpm) );
 
  setAllChildrenVisible(m_Expanded);
}

void QmitkListViewExpanderIcon::mouseReleaseEvent ( QMouseEvent * e )
{
  setExpanded(!m_Expanded); //toggle status
}

void QmitkListViewExpanderIcon::showEvent(QShowEvent* e)
{
  setAllChildrenVisible(true);
  QLabel::showEvent(e);
}

void QmitkListViewExpanderIcon::hideEvent(QHideEvent* e)
{
  setAllChildrenVisible(false);
  QLabel::hideEvent(e);
}

void QmitkListViewExpanderIcon::setAllChildrenVisible(bool visible)
{
  // show/hide all children
  for (std::list<QWidget*>::iterator iter = m_Children.begin();
       iter != m_Children.end();
       ++iter)
  {
    if (visible)
      (*iter)->show();
    else
      (*iter)->hide();
  }
}

