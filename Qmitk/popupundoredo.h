#ifndef POPUPUNDOREDO_H
#define POPUPUNDOREDO_H

#include "popupactionhistory.h"

class QString;
class QMouseEvent;
class QCloseEvent;

/// A subclass of PopupListBox, implementing the behaviour of a MS Word undo/redo list
class PopupUndoRedoListBox : public PopupActionHistory
{
  Q_OBJECT
public:
  PopupUndoRedoListBox(QWidget* parent = 0, const char*  name=0);
  void setInfoPhrases(const QString& initialPhrase, const QString& mouseMovePhrase);
  
  virtual void popup(QWidget* parent = 0);  /// Call to popup this widget. parent determines popup position

signals:
  void selectedRange(int); /// Emitted, when popup is closed and one or more items are selected. Argument provides number of selected items.

protected:
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void closeEvent(QCloseEvent*);

private:
  int m_num_selected;
  QString m_initialPhrase;
  QString m_mouseMovePhrase;
};

#endif
