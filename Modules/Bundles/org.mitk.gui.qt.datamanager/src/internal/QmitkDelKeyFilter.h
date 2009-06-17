#ifndef QMITKDELKEYFILTER_H_
#define QMITKDELKEYFILTER_H_

#include <QObject>

///
/// A small class which "eats" all Del-Key-pressed events on the node table.
/// When the Del Key is pressed selected nodes should be removed.
///
class QmitkDelKeyFilter : public QObject
{
  Q_OBJECT
public:
  QmitkDelKeyFilter(QObject* _DataManagerView = 0);
protected:
  bool eventFilter(QObject *obj, QEvent *event);
};

#endif // QMITKDELKEYFILTER_H_