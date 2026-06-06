/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMorphologicalOperationsWidget_h
#define QmitkMorphologicalOperationsWidget_h

#include <MitkSegmentationUIExports.h>

#include <mitkMorphologicalOperations.h>
#include <mitkRenderingManager.h>

#include <QmitkAbstractNodeSelectionWidget.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkMorphologicalOperationsWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
}

/**
 * \brief Widget providing morphological operations on segmentation labels.
 *
 * Offers buttons for closing, opening, dilation, erosion, and fill-holes operations.
 * The user can select a segmentation and label, choose the structural element type
 * (ball or cross) and its radius, then apply the operation.
 *
 * \sa mitk::MorphologicalOperations
 */
class MITKSEGMENTATIONUI_EXPORT QmitkMorphologicalOperationsWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the widget with GUI elements and signal/slot connections.
   * \param[in] dataStorage Pointer to the data storage for node selection.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkMorphologicalOperationsWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);
  /** \brief Destructor. */
  ~QmitkMorphologicalOperationsWidget() override;

public slots:
  /** \brief Performs morphological closing on the selected label. */
  void OnClosingButtonClicked();
  /** \brief Performs morphological opening on the selected label. */
  void OnOpeningButtonClicked();
  /** \brief Performs morphological dilation on the selected label. */
  void OnDilatationButtonClicked();
  /** \brief Performs morphological erosion on the selected label. */
  void OnErosionButtonClicked();
  /** \brief Fills holes in the selected label. */
  void OnFillHolesButtonClicked();
  /** \brief Called when the segmentation selection changes. */
  void OnSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList nodes);
  /** \brief Called when the structural element radio buttons are clicked. */
  void OnRadioButtonsClicked();

protected:
  void ConfigureButtons();

  using MorphFunctionType = void(mitk::Image::Pointer& image, int factor,
    mitk::MorphologicalOperations::StructuralElementType structuralElement);
  void Processing(std::function<MorphFunctionType> morphFunction, const std::string& opsName) const;

private:
  mitk::Image::Pointer GetSelectedLabelMask() const;
  /**
   * \brief Writes the operation result back (new label or in-place) and records its provenance.
   * \param[in] resultMask The mask produced by the morphological operation.
   * \param[in] labelName Human-readable display name for the (new) label.
   * \param[in] provenanceOpName Algorithm-provenance name recorded via Label::AddToolUse
   *            (e.g. "Morphological Closing"); must contain no "|" or ": " separator.
   */
  void SaveResultLabelMask(
    const mitk::Image* resultMask, const std::string& labelName, const std::string& provenanceOpName) const;

  mitk::MorphologicalOperations::StructuralElementType CreateStructerElement_UI() const;

  std::unique_ptr<Ui::QmitkMorphologicalOperationsWidgetControls> m_Controls;
};

#endif
