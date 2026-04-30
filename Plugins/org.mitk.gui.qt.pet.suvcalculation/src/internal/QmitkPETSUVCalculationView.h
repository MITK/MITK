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

#ifndef QmitkPETSUVCalculationView_h
#define QmitkPETSUVCalculationView_h

#include <mitkImage.h>
#include <mitkSUVCalculationHelper.h>
#include <memory>

namespace Ui
{
  class QmitkPETSUVCalculationViewControls;
}

#include <QmitkAbstractView.h>

#include <QString>
#include <QStyledItemDelegate>

/**
* @brief Custom tree model for displaying decay time data
*
* This model handles both auto and user - defined modes :
*-Auto mode : Shows hierarchical structure(time steps->slices)
* -User - defined mode : Shows flat structure with single decay time per time step
*/
/**
* @brief Custom tree model for displaying decay time data
*
* This model handles both auto and user - defined modes :
*-Auto mode : Shows hierarchical structure(time steps->slices)
* -User - defined mode : Shows flat structure with single decay time per time step
*/
class DecayTimeMapModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum class Mode { Auto, UserDefined };

  explicit DecayTimeMapModel(QObject* parent = nullptr);

  void SetDecayTimeMap(const mitk::DecayTimeMapType& map);
  void SetMode(Mode mode);
  Mode GetMode() const;

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
  mitk::DecayTimeMapType m_DecayTimeMap;
  Mode m_Mode = Mode::Auto;

  bool hasSingleTimeStep() const;

  std::optional<mitk::TimeStepType> GetTimeStep(const QModelIndex& index) const;
  std::optional<mitk::SlicedData::IndexValueType> GetSliceIndex(const QModelIndex& index) const;
};

/**
 * @brief Custom delegate for editing decay time values
 *
 * Provides a double spin box editor for decay time values when in user-defined mode
 */
class DecayTimeDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit DecayTimeDelegate(QObject* parent = nullptr);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
    const QModelIndex& index) const override;
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

private:
  static constexpr double MIN_DECAY_TIME = 0.0;
  static constexpr double MAX_DECAY_TIME = 1000000.0; // 1 million seconds
  static constexpr int DECIMALS = 6;
  static constexpr double SINGLE_STEP = 0.1;
};

/*!
 *	@brief Test Plugin for SUV calculations of PET images
 */
class QmitkPETSUVCalculationView : public QmitkAbstractView
{
  Q_OBJECT

public:
  /*! @brief The view's unique ID - required by MITK */
  static const std::string VIEW_ID;

  QmitkPETSUVCalculationView();
  ~QmitkPETSUVCalculationView() override;

protected slots:

  void OnCalculateSUVButtonClicked();
  void OnNuclideLookupClicked();
  void OnCheckPETOnlyToggled(bool);

  void OnInjectedActivityChanged(double);
  void OnBodyWeightChanged(double);
  void OnHalfLifeChanged(double);
  void OnTimeToMeasurementChanged(int);
  void OnPETSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

protected:
  // Overridden base class functions

  /*!
   *	@brief					Sets up the UI controls and connects the slots and signals. Gets
   *							called by the framework to create the GUI at the right time.
   *	@param[in,out] parent	The parent QWidget, as this class itself is not a QWidget
   *							subclass.
   */
  void CreateQtPartControl(QWidget *parent) override;

  /*!
   *	@brief	Sets the focus to the plot curve button. Gets called by the framework to set the
   *			focus on the right widget.
   */
  void SetFocus() override;

  /**updates the widgets according to the internal values.*/
  void UpdateWidgets();

  /**Function populates the nuclide half life map.*/
  void GenerateHalfLifeMap();

  mitk::Image::Pointer CalcSUV(mitk::Image *inputImage) const;

  // Variables

  /*! @brief The view's UI controls */
  std::unique_ptr<Ui::QmitkPETSUVCalculationViewControls> m_Controls;

private:
  /**Activity in Bq*/
  double m_injectedActivity;

  /**Weight in kg*/
  double m_bodyweight;

  /** Time between injection and image acquisition in sec. Used when defined by user and not autodetected.*/
  bool m_validAutoTime;
  mitk::DecayTimeMapType m_autoDecayTime;
  /** DICOM decay-correction strategy detected on the selected node. Reset on each new selection. */
  mitk::DecayCorrectionStrategy m_DecayStrategy = mitk::DecayCorrectionStrategy::None;

  /** Half life in sec*/
  double m_halfLife;

  /**The predefined nuclide by the image data. Empty string implies no definition/custom value.*/
  std::string m_DefinedNuclide;

  typedef std::map<std::string, double> HalfLifeMapType;

  HalfLifeMapType m_HalfLifeMap;

  /** Helper flag that helps to prevent recursive triggering in the gui logic.*/
  bool m_internalUpdate;

  std::unique_ptr<DecayTimeMapModel> m_decayTimeModel;

  QWidget *m_ParentWidget;
};

#endif
