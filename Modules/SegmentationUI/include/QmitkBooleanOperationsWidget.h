/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBooleanOperationsWidget_h
#define QmitkBooleanOperationsWidget_h

#include <mitkLabelSetImage.h>

#include <QmitkAbstractNodeSelectionWidget.h>

#include <QWidget>

#include <MitkSegmentationUIExports.h>
#include <memory>

namespace Ui
{
  class QmitkBooleanOperationsWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
}

/**
 * \brief Widget providing boolean operations (difference, intersection, union) on multi-label segmentation labels.
 *
 * Allows the user to select a segmentation and two labels, then apply boolean set operations
 * (difference, intersection, or union) to produce a result label mask. The result is saved back
 * into the segmentation.
 *
 * \sa QmitkMultiLabelInspector
 */
class MITKSEGMENTATIONUI_EXPORT QmitkBooleanOperationsWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the boolean operations widget.
   * \param[in] dataStorage Pointer to the data storage for node selection.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkBooleanOperationsWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkBooleanOperationsWidget() override;

private slots:
  void OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList nodes);
  void OnLabelSelectionChanged(mitk::MultiLabelSegmentation::LabelValueVectorType labels);
  void OnDifferenceButtonClicked();
  void OnIntersectionButtonClicked();
  void OnUnionButtonClicked();

private:
  void ConfigureWidgets();
  /**
   * \brief Adds the operation result as a new label in a new group and records its provenance.
   * \param[in] resultMask The binary mask produced by the boolean operation.
   * \param[in] labelName Human-readable display name for the new label.
   * \param[in] provenanceOpName Algorithm-provenance name recorded via Label::AddToolUse
   *            (e.g. "Boolean Union"); must contain no "|" or ": " separator.
   */
  void SaveResultLabelMask(
    const mitk::Image* resultMask, const std::string& labelName, const std::string& provenanceOpName) const;


  std::unique_ptr<Ui::QmitkBooleanOperationsWidgetControls> m_Controls;
};

#endif
