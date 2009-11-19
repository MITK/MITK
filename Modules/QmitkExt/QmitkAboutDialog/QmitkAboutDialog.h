
#include <QDialog>
#include <QWidget>
#include <mitkCommon.h>
#include <ui_QmitkAboutDialogGUI.h>



class QMITKEXT_EXPORT QmitkAboutDialog : public QDialog
{
  Q_OBJECT

  public:

    QmitkAboutDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkAboutDialog();

  protected slots:


  protected:

    Ui::QmitkAboutDialog* m_GUI;
};
