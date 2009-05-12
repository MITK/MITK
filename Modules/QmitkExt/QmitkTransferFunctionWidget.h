
#ifndef QMITKTRANSFERFUNCTIONWIDGET_H
#define QMITKTRANSFERFUNCTIONWIDGET_H

#include "ui_QmitkTransferFunctionWidget.h"

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataTreeNode.h>

class QMITK_EXPORT QmitkTransferFunctionWidget : public QWidget, public Ui::QmitkTransferFunctionWidget
{

	Q_OBJECT

 public:

   QmitkTransferFunctionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
   ~QmitkTransferFunctionWidget () ;

   virtual void AddPoints();
   virtual void ResetTF();
   virtual void ChooseCS(int cstyle);
   virtual void ResetCS();
   virtual void AddCS();

	public slots:

	 void SetDataTreeNode(mitk::DataTreeNode* node);
	 void UpdateMinMaxLabels();
	 void ChooseTF(int choice);
	 void ImmediateUpdate(bool state);
	 void SetXValue();
	 void SetYValue();

   /** */
   void OnChangeTransferFunctionMode( int mode );

signals:

   /** */
   void SignalTransferFunctionModeChanged( int );

 private:

	 int number_c; //Anzahl Color-Points
	 int number_s; //Anzahl Scalar-Points
	 double color_pointsX[50];
	 double color_pointsR[50];
	 double color_pointsG[50];
	 double color_pointsB[50];
	 double scalar_pointsX[200];
	 double scalar_pointsY[200];


};
#endif
