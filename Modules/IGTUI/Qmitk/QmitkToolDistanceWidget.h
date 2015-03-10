/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _QmitkToolDistanceWidget_H_INCLUDED
#define _QmitkToolDistanceWidget_H_INCLUDED

#include "ui_QmitkToolDistanceWidgetControls.h"
#include "MitkIGTUIExports.h"


#include <itkProcessObject.h>
#include <QLabel>
#include <QVector>




/*!
\brief QmitkToolDistanceWidget

Widget for setting up and controlling an update timer in an IGT-Pipeline.

*/
class MITKIGTUI_EXPORT QmitkToolDistanceWidget : public QWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public:

  typedef QVector<QVector<QLabel*> > DistanceLabelType;

  /*!
  \brief default constructor
  */
  QmitkToolDistanceWidget( QWidget* parent );

  /*!
  \brief default destructor
  */
  virtual ~QmitkToolDistanceWidget();

  /*!
  \brief This method displays the matrix with the distances between the tracking source's outputs in a QGridLayout.
  */
  void ShowDistanceValues(itk::ProcessObject::DataObjectPointerArray & outputs);

  /*!
  \brief This method creates the initial distances matrix and labels it with the connected tool names.
  */
  void CreateToolDistanceMatrix(itk::ProcessObject::DataObjectPointerArray & outputs);

  /*!
  \brief This method clears the whole tool distances matrix
  */
  void ClearDistanceMatrix();

public slots:
  /*!
  \brief This method set's all distance entries in the matrix to "---". Can be used e.g. if tracking is stopped.
  */
  void SetDistanceLabelValuesInvalid();

protected:
  void CreateConnections();
  void CreateQtPartControl( QWidget *parent );
  Ui::QmitkToolDistanceWidgetControls* m_Controls;  ///< gui widgets


private:
  /*!
  \brief Double vector for all between tool distances labels.
  */
  DistanceLabelType* m_DistanceLabels;

};
#endif // _QmitkToolDistanceWidget_H_INCLUDED

