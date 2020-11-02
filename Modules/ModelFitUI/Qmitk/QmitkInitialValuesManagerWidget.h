/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITK_INITIAL_VALUES_MANAGER_WIDGET_H
#define QMITK_INITIAL_VALUES_MANAGER_WIDGET_H

#include "MitkModelFitUIExports.h"

#include "ui_QmitkInitialValuesManagerWidget.h"
#include <QWidget>

#include "mitkModelTraitsInterface.h"
#include "mitkInitialParameterizationDelegateBase.h"

/*forward declarations*/
class QmitkInitialValuesModel;
class QmitkInitialValuesTypeDelegate;
class QmitkInitialValuesDelegate;

namespace mitk
{
  class DataStorage;
  class BaseGeometry;
}

/**
* \class QmitkInitialValuesManagerWidget
* \brief Widget that allows to edit the initial values of an model.
*/
class MITKMODELFITUI_EXPORT QmitkInitialValuesManagerWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkInitialValuesManagerWidget(QWidget* parent = nullptr);
  ~QmitkInitialValuesManagerWidget() override;

  /** Returns the current set initial values of the model.*/
  mitk::ModelTraitsInterface::ParametersType getInitialValues() const;
  mitk::InitialParameterizationDelegateBase::Pointer getInitialParametrizationDelegate() const;

  bool hasValidInitialValues() const;

signals:
  void initialValuesChanged();

public Q_SLOTS:
  /** Sets the names and the values of the initial parameter set for the model.
   @param names List of all possible parameter names. It is assumed that the
   index of the list equals the parameter index in the respective fitting model and its parameter values.
   @param values Default values to start with.*/
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names,
                        const mitk::ModelTraitsInterface::ParametersType values);
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names);

  void setDataStorage(mitk::DataStorage* storage);

  void setReferenceImageGeometry(mitk::BaseGeometry* refgeo);

protected:

  QmitkInitialValuesModel* m_InternalModel;

  QmitkInitialValuesTypeDelegate* m_TypeDelegate;
  QmitkInitialValuesDelegate* m_ValuesDelegate;

  Ui::QmitkInitialValuesManagerWidget m_Controls;

protected Q_SLOTS:
  void OnModelReset();

};

#endif // QmitkInitialValuesManagerWidget_H
