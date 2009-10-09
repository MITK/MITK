#ifndef QMITKNODETABLEVIEWKEYFILTER_H_
#define QMITKNODETABLEVIEWKEYFILTER_H_

#include "cherryIPreferencesService.h"

#include <QObject>

///
/// A small class which "eats" all Del-Key-pressed events on the node table.
/// When the Del Key is pressed selected nodes should be removed.
///
class QmitkNodeTableViewKeyFilter : public QObject
{
  Q_OBJECT
public:
  QmitkNodeTableViewKeyFilter(QObject* _DataManagerView = 0);
protected:
  bool eventFilter(QObject *obj, QEvent *event);

  ///
  /// The Preferences Service to retrieve and store preferences.
  ///
  cherry::IPreferencesService::WeakPtr m_PreferencesService;
};

#endif // QMITKNODETABLEVIEWKEYFILTER_H_
