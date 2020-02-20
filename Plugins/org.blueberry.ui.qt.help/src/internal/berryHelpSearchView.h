/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  ~HelpSearchView() override;

  void SetFocus() override;

protected:

  void CreateQtPartControl(QWidget* parent) override;

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

  bool eventFilter(QObject* o, QEvent *e) override;
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
