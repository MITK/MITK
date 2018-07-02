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

#ifndef QmitkInitialValuesModel_h
#define QmitkInitialValuesModel_h

#include <QAbstractTableModel>

#include "mitkSimpleBarrierConstraintChecker.h"
#include "mitkModelTraitsInterface.h"
#include "mitkDataNode.h"
#include "mitkInitialParameterizationDelegateBase.h"

#include "MitkModelFitUIExports.h"


/*!
\class QmitkInitialValuesModel
Model that handles the definition of inital model values.
*/
class MITKMODELFITUI_EXPORT QmitkInitialValuesModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  QmitkInitialValuesModel(QObject* parent = NULL);
  virtual ~QmitkInitialValuesModel() {};

  /** Sets the names and the values of the initial parameter set for the model.
   @param names List of all possible parameter names. It is assumed that the
   index of the list equals the parameter index in the respective fitting model and its parameter values.
   @values Default values to start with.*/
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names,
                        const mitk::ModelTraitsInterface::ParametersType values);
  /**@overload
   Convinience method that sets the default initial values always to zero.*/
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names);

  /** Adds an image as a source for the initial value of a parameter.
  * @param node Pointer to the image that is the value source.
  * @param paramIndex Indicates which parameter is defined by the source image.
  * It equals the position of the vector defined by setInitialValues().
  * @remark setting an image for an index overwrites the value for this index set by
  * SetInitialParameterization.
  * @pre paramIndex must be in bound of the initial parametrization vector.
  * @pre node must be a valid image instance*/
  void addInitialParameterImage(const mitk::DataNode* node, mitk::ModelTraitsInterface::ParametersType::size_type paramIndex);

  bool hasValidInitialValues() const;

  void resetInitialParameterImage();

  /** Returns a pointer to a delegate instance that represents the parameterization of the model.*/
  mitk::InitialParameterizationDelegateBase::Pointer getInitialParametrizationDelegate() const;
  /** Returns the current set initial values of the model.
   * @Remark: this are only the simpel scalar initial values. If an source image was set, this is missed here.
   * Use getInitialParametrizationDelegate() to get everything at once.*/
  mitk::ModelTraitsInterface::ParametersType getInitialValues() const;

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  /**Indicates if the content of the model was modified since the data was set via setInitialValues()*/
  bool isModified();

private:
  int valueType(const QModelIndex& index) const;

  mitk::ModelTraitsInterface::ParametersType m_Values;
  mitk::ModelTraitsInterface::ParameterNamesType m_ParameterNames;

  typedef std::map<mitk::ModelTraitsInterface::ParametersType::size_type, mitk::DataNode::ConstPointer> ImageMapType;
  ImageMapType m_ParameterImageMap;

  /** Indicates if the data of the model was modified, since the model was set. */
  bool m_modified;

};

#endif // QmitkInitialValuesModel_h

