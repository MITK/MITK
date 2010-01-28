
#include <QDialog>
#include "QmitkExtExports.h"
#include <QWidget>
#include <mitkCommon.h>


class QmitkExt_EXPORT QmitkAboutDialog : public QDialog
{
  Q_OBJECT

  public:

    QmitkAboutDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkAboutDialog();

};
