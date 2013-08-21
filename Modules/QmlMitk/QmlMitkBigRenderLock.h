#ifndef QmlMitkBigRenderLock_h
#define QmlMitkBigRenderLock_h

#include <QtCore>

#include "QmlMitkExports.h"

class QmlMitk_EXPORT QmlMitkBigRenderLock : public QObject
{
  Q_OBJECT
  public:

    static QMutex& GetMutex();

    QmlMitkBigRenderLock(QObject* parent = 0);

  protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif
