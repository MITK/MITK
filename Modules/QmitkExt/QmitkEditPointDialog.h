#ifndef QmitkEditPointDialog_h
#define QmitkEditPointDialog_h

#include "QmitkExtExports.h"

#include <QDialog>
#include <mitkPointSet.h>

struct QmitkEditPointDialogData;
/*!
 * \brief A dialog for editing points directly (coordinates) via TextEdits
 *
 */
class QmitkExt_EXPORT QmitkEditPointDialog : public QDialog
{
  Q_OBJECT

  public:
    QmitkEditPointDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    virtual ~QmitkEditPointDialog();

    void SetPoint( mitk::PointSet* _PointSet, mitk::PointSet::PointIdentifier _PointId, int timestep=0 );
  protected slots:
    void OnOkButtonClicked( bool triggered = false );
  protected:
    QmitkEditPointDialogData* d;
};

#endif // QmitkEditPointDialog_h