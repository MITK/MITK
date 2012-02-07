/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYHELPSEARCHVIEW_H_
#define BERRYHELPSEARCHVIEW_H_

#include <berryQtViewPart.h>

class QHelpSearchEngine;
class QHelpSearchResultWidget;
class QHelpSearchQueryWidget;

namespace berry {

class HelpSearchView : public QtViewPart
{
  Q_OBJECT

public:

  HelpSearchView();
  ~HelpSearchView();

  void SetFocus();

protected:

  void CreateQtPartControl(QWidget* parent);

private Q_SLOTS:

  void zoomIn();
  void zoomOut();
  void resetZoom();

  void search() const;
  void searchingStarted();
  void searchingFinished(int hits);

  void showContextMenu(const QPoint& pos);
  void requestShowLink(const QUrl& link);

private:

  bool eventFilter(QObject* o, QEvent *e);
  void keyPressEvent(QKeyEvent *keyEvent);

private:

  Q_DISABLE_COPY(HelpSearchView)

  int m_ZoomCount;
  QWidget* m_Parent;
  QHelpSearchEngine* m_SearchEngine;
  QHelpSearchResultWidget* m_ResultWidget;
  QHelpSearchQueryWidget* m_QueryWidget;

};

} // namespace berry

#endif /*BERRYHELPSEARCHVIEW_H_*/
