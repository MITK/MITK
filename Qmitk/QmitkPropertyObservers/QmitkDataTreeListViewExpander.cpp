#include <QmitkDataTreeListViewExpander.h>

#include <qpixmap.h>
#include <qobjectlist.h>
#include <qlayout.h>

#include <stdexcept>

#include "collapsed.xpm"
#include "expanded.xpm"

// for debugging
#include <iostream>

QmitkListViewExpanderIcon::QmitkListViewExpanderIcon(const QGridLayout* childContainer, QWidget* parent, const char* name )
: QLabel(parent, name),
  m_Expanded(true),
  m_ChildContainer(childContainer)
{
  if (!childContainer) throw std::invalid_argument("NULL pointer for childContainer makes no sense in QmitkListViewExpanderIcon()");
  setExpanded(m_Expanded);
}

QmitkListViewExpanderIcon::~QmitkListViewExpanderIcon()
{
}

void QmitkListViewExpanderIcon::addWidget(QWidget* widget)
{
  if (widget)
    m_Children.push_back(widget);
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

