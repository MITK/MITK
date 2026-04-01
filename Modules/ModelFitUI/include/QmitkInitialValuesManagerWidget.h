/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkInitialValuesManagerWidget_h
#define QmitkInitialValuesManagerWidget_h

#include <MitkModelFitUIExports.h>

#include <QWidget>

#include <mitkModelTraitsInterface.h>
#include <mitkInitialParameterizationDelegateBase.h>
#include <mitkNodePredicateBase.h>
#include <memory>

/*forward declarations*/
class QmitkInitialValuesModel;
class QmitkInitialValuesTypeDelegate;
class QmitkInitialValuesDelegate;

namespace mitk
{
  class DataStorage;
  class BaseGeometry;
}

namespace Ui
{
  class QmitkInitialValuesManagerWidget;
}

/**
 * \class QmitkInitialValuesManagerWidget
 * \brief Widget that allows editing the initial parameter values of a model fit.
 *
 * This widget provides a table view for configuring the initial values used in model
 * fitting. Each parameter can be initialized either with a scalar value or with an
 * image from the data storage. The widget manages the appropriate delegates for type
 * selection and value editing.
 *
 * When a reference image geometry is provided, the image selection is filtered to only
 * show 3D images with matching geometry.
 *
 * \sa QmitkInitialValuesModel
 * \sa QmitkInitialValuesDelegate
 * \sa QmitkInitialValuesTypeDelegate
 */
class MITKMODELFITUI_EXPORT QmitkInitialValuesManagerWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a QmitkInitialValuesManagerWidget.
   * \param[in] parent Optional parent widget.
   */
  QmitkInitialValuesManagerWidget(QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkInitialValuesManagerWidget() override;

  /**
   * \brief Returns the current scalar initial values of the model parameters.
   *
   * \return The parameter values vector.
   * \note This only returns scalar initial values. If an image source was set for a
   *       parameter, it is not reflected here. Use getInitialParametrizationDelegate()
   *       to retrieve the complete parameterization.
   */
  mitk::ModelTraitsInterface::ParametersType getInitialValues() const;

  /**
   * \brief Creates and returns a parameterization delegate representing the full initial configuration.
   *
   * The returned delegate encapsulates both scalar values and image-based parameterizations.
   *
   * \return Smart pointer to an InitialParameterizationDelegateBase instance.
   */
  mitk::InitialParameterizationDelegateBase::Pointer getInitialParametrizationDelegate() const;

  /**
   * \brief Checks whether all initial values are valid.
   *
   * Returns false if any image-based parameter has a null data node.
   *
   * \return True if all initial values are valid, false otherwise.
   */
  bool hasValidInitialValues() const;

signals:
  /** \brief Emitted whenever the initial values are modified by the user or reset programmatically. */
  void initialValuesChanged();

public Q_SLOTS:
  /**
   * \brief Sets the parameter names, default values, and units for the model.
   *
   * \param[in] names List of parameter names. The index corresponds to the parameter
   *            index in the fitting model.
   * \param[in] values Default initial values for the parameters.
   * \param[in] units Map of parameter names to their unit strings.
   *
   * \pre names, values, and units must have the same size.
   */
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names,
                        const mitk::ModelTraitsInterface::ParametersType values,
                        const mitk::ModelTraitsInterface::ParamterUnitMapType units);

  /**
   * \brief Sets the parameter names with default initial values of zero.
   * \param[in] names List of parameter names.
   */
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names);

  /**
   * \brief Sets the data storage used for image-based initial value selection.
   *
   * Also resets any currently selected initial parameter images.
   *
   * \param[in] storage Pointer to the DataStorage instance.
   */
  void setDataStorage(mitk::DataStorage* storage);

  /**
   * \brief Sets the reference image geometry used to filter selectable images.
   *
   * When a valid geometry is provided, only 3D images matching that geometry are
   * offered for selection. When nullptr is passed, all 3D images are available.
   * Resets any currently selected initial parameter images.
   *
   * \param[in] refgeo Pointer to the reference geometry, or nullptr to clear the filter.
   */
  void setReferenceImageGeometry(mitk::BaseGeometry* refgeo);

protected:

  std::unique_ptr<Ui::QmitkInitialValuesManagerWidget> m_Controls;
  QmitkInitialValuesModel* m_InternalModel;

  QmitkInitialValuesTypeDelegate* m_TypeDelegate;
  QmitkInitialValuesDelegate* m_ValuesDelegate;

  mitk::NodePredicateBase::Pointer m_NoHiddenOrHelperPredicate;

protected Q_SLOTS:
  void OnModelReset();

};

#endif
