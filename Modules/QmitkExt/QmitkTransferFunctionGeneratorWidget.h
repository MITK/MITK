
#ifndef QMITKTRANSFERFUNCTIONGENERATORWIDGET_H
#define QMITKTRANSFERFUNCTIONGENERATORWIDGET_H

#include "ui_QmitkTransferFunctionGeneratorWidget.h"

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataTreeNode.h>
#include <mitkTransferFunctionProperty.h>

class QMITKEXT_EXPORT QmitkTransferFunctionGeneratorWidget : public QWidget, public Ui::QmitkTransferFunctionGeneratorWidget
{

  Q_OBJECT

  public:

    QmitkTransferFunctionGeneratorWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkTransferFunctionGeneratorWidget () ;

	  void SetDataTreeNode(mitk::DataTreeNode* node);
   
  public slots:

    void OnSavePreset( );
    void OnLoadPreset( );
    void OnMitkInternalPreset( int mode );
    
    void OnDeltaLevelWindow( int dx, int dy );
    void OnDeltaThreshold( int dx, int dy );

  signals:

    void SignalTransferFunctionModeChanged( int );
    void SignalUpdateCanvas( );
   
  protected:
 
    mitk::TransferFunctionProperty::Pointer tfpToChange;
    
    int histoMinimum;
    int histoMaximum;
    
    int thPos;
    int thDelta;
    
    const mitk::Image::HistogramType *histoGramm;
    
    QString presetFileName;
    
};
#endif
