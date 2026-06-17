/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
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
#include <mitkSUVImageFilter.h>
#include <memory>

namespace Ui
{
  class QmitkPETSUVCalculationViewControls;
}

#include <QmitkAbstractView.h>

#include <QString>
#include <QStyledItemDelegate>

/**
 * \brief Custom tree model for displaying decay time data.
 *
 * Three display modes:
 *  - Auto:        hierarchical structure (time steps -> slices) reflecting
 *                 the per-(timestep, slice) decay times the filter resolved.
 *                 Read-only.
 *  - UserDefined: flat structure showing the uniform user-defined decay time
 *                 per time step. Read-only (the spinbox is the authority).
 *  - PerSlice:    hierarchical (time steps -> slices), editable. Each cell
 *                 write emits dataChanged so the view can push the full map
 *                 to the filter.
 */
class QmitkDecayTimeMapModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum class Mode { Auto, UserDefined, PerSlice };

  explicit QmitkDecayTimeMapModel(QObject* parent = nullptr);

  void SetDecayTimeMap(const mitk::DecayTimeMapType& map);
  const mitk::DecayTimeMapType& GetDecayTimeMap() const;
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
 * \brief Delegate providing a double spin-box editor for the decay-time
 *        column in per-slice mode.
 */
class QmitkDecayTimeDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkDecayTimeDelegate(QObject* parent = nullptr);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
    const QModelIndex& index) const override;
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

private:
  static constexpr double MIN_DECAY_TIME = 0.0;
  static constexpr double MAX_DECAY_TIME = 1000000.0;
  static constexpr int    DECIMALS       = 6;
  static constexpr double SINGLE_STEP    = 0.1;
};

/*!
 *  \brief Plugin view for SUV calculations of PET images.
 *
 *  The view drives a single \c mitk::SUVImageFilter. Every UI control is
 *  either an override slot on that filter (weight, activity, ...) or a
 *  configuration flag (variant, strict-DICOM, force-units). Whenever a
 *  control changes, the filter is reconfigured against the currently
 *  selected PET image and the resulting effective values / diagnostics
 *  are rendered back into the UI.
 */
class QmitkPETSUVCalculationView : public QmitkAbstractView
{
  Q_OBJECT

public:
  QmitkPETSUVCalculationView();
  ~QmitkPETSUVCalculationView() override;

protected slots:

  void OnCalculateSUVButtonClicked();

  void OnCheckPETOnlyToggled(bool);

  void OnVariantChanged(int);
  void OnPatientHeightChanged(double);
  void OnPatientSexChanged(int);

  void OnInjectedActivityChanged(double);
  void OnBodyWeightChanged(double);
  void OnHalfLifeChanged(double);
  void OnNuclideComboChanged(int);

  void OnTimeToMeasurementChanged(int);
  void OnDecayTimeRadioToggled();
  void OnPerSliceDecayMapEdited(const QModelIndex&, const QModelIndex&, const QList<int>&);

  void OnTracerIndexChanged(int);

  void OnForcePETToggled(bool);
  void OnForceBqMlToggled(bool);
  void OnStrictDicomToggled(bool);

  void OnPETSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

protected:
  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

  /** Refresh every UI control from the filter's current state. */
  void UpdateWidgets();

  /** Populate \c m_HalfLifeMap and the nuclide combo. */
  void GenerateHalfLifeMap();

  std::unique_ptr<Ui::QmitkPETSUVCalculationViewControls> m_Controls;

private:
  /** SUV computation backend. Holds user overrides as std::optional slots
   *  and derives the rest from the input image's DICOM properties.
   *  Replaced (not just reset) on every selection so previous overrides
   *  cannot bleed across nodes. */
  mitk::SUVImageFilter::Pointer m_Filter;

  /** Description of the most recent ConfigureFromProperties failure, or
   *  empty when the filter is configured. */
  std::string m_LastConfigError;

  /** UI-friendly per-category guidance text derived from the typed
   *  exception. Drives \c diagnosticsWidget independently of the raw
   *  exception description in \c m_LastConfigError. */
  std::string m_LastConfigActionable;

  /** \c true when the most recent configuration problem stems from decay
   *  timing (ambiguous auto-detection, or an invalid / conflicting manual
   *  override). Lets the time group's strategy line surface the actionable
   *  warning in red, where it is harder to miss than in the compact
   *  diagnostics box. */
  bool m_DecayTimingProblem = false;

  /** Mirrors m_Filter's internal "configured" state. */
  bool m_Configured = false;

  /** \c true if the current input's Radiopharmaceutical Information
   *  Sequence holds more than one item. Drives the visibility of the
   *  tracer row (\c tracerCombo). */
  bool m_MultiTracerDetected = false;

  /** Sentinel for "custom" entry in the nuclide combo (i.e. the half-life
   *  was edited manually and no longer maps to a named nuclide). */
  static const QString NUCLIDE_CUSTOM_LABEL;

  using HalfLifeMapType = std::map<std::string, double>;
  HalfLifeMapType m_HalfLifeMap;

  QmitkDecayTimeMapModel* m_decayTimeModel = nullptr;

  QWidget* m_ParentWidget = nullptr;

  /** Re-run \c ConfigureFromProperties on the currently selected image
   *  and refresh \c m_LastConfigError / \c m_LastConfigActionable. */
  void ReconfigureFilterFromCurrentImage();

  /** Read the multi-tracer state of the currently selected image and
   *  repopulate \c tracerCombo. Called once per selection. */
  void DetectAndPopulateTracers();

  /** Best-effort lookup of the auto-detected nuclide name (from the
   *  current input's radiopharmaceutical info) and selection of the
   *  matching entry in \c nuclideCombo. */
  void RefreshNuclideComboFromImage();

  /** Push the model's current per-slice map to the filter, reconfigure,
   *  and refresh the UI. Used when the user edits a cell in PerSlice
   *  mode and when the radio is toggled to PerSlice. */
  void PushPerSliceMapToFilter();

  /** Seed a complete per-(timestep, slice) decay-time map from the
   *  filter's current effective decay correction. Returns an empty map
   *  if the filter is not configured. */
  mitk::DecayTimeMapType SeedPerSliceMapFromEffective() const;

  /** Read the currently selected image from \c petNodeSelector, or
   *  return \c nullptr if none is selected / not an Image. */
  mitk::Image* CurrentInputImage() const;

  /** Compose the "Auto-detected" summary text shown in the diagnostics
   *  widget: pixel semantics, vendor, activity / prenorm scale, decay
   *  strategy, nuclide, tracer index, force-flags. Returns an empty
   *  string if no image is selected; if an image is selected but
   *  configuration failed, returns the partial auto-detected fields
   *  plus a failure note. */
  QString BuildDetectedInfoText() const;
};

#endif
