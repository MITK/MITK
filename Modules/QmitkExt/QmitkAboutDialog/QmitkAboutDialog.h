
#include <QDialog>
#include <QWidget>
#include <mitkCommon.h>


class QMITKEXT_EXPORT QmitkAboutDialog : public QDialog
{
  Q_OBJECT

  public:

    QmitkAboutDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkAboutDialog();

};
