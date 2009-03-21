#ifndef QMITKVOLUMERENDERINGVIEW_H_
#define QMITKVOLUMERENDERINGVIEW_H_

#include <QObject>
#include <QmitkFunctionality.h>
#include <QmitkDataStorageListModel.h>
#include <QmitkTransferFunctionWidget.h>

class QmitkVolumeRenderingView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

public:

  void SetFocus();

  ~QmitkVolumeRenderingView();

protected:

  void CreateQtPartControl(QWidget* parent);

protected slots:

  void comboIndexChanged(int index);

private:

  QmitkDataStorageListModel* m_ComboModel;
  QmitkTransferFunctionWidget* m_TransferFunctionWidget;

};

#endif /*QMITKVOLUMERENDERINGVIEW_H_*/
