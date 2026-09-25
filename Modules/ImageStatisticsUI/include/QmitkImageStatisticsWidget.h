/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkImageStatisticsWidget_h
#define QmitkImageStatisticsWidget_h

#include <MitkImageStatisticsUIExports.h>

#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkImageStatisticsContainer.h>

#include <QWidget>
#include <memory>

class QSortFilterProxyModel;
class QmitkImageStatisticsTreeModel;

namespace Ui
{
  class QmitkImageStatisticsControls;
}

/**
 * \brief Widget for displaying image statistics in a tree view with clipboard export.
 *
 * This widget wraps a QmitkImageStatisticsTreeModel in a sortable tree view and provides
 * controls for ignoring zero-valued voxels and copying statistics to the clipboard.
 * It automatically enables its controls when statistics data becomes available.
 *
 * \sa QmitkImageStatisticsTreeModel
 * \sa QmitkHistogramVisualizationWidget
 * \sa QmitkStatisticsModelToStringConverter
 */
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the image statistics widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkImageStatisticsWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkImageStatisticsWidget() override;

  /**
   * \brief Sets the data storage from which the model fetches statistics objects.
   * \param[in] newDataStorage Pointer to the data storage. Must be valid.
   * \pre The data storage must not be nullptr.
   */
  void SetDataStorage(mitk::DataStorage *newDataStorage);

  /**
   * \brief Sets the image nodes whose statistics should be displayed.
   * \param[in] nodes Vector of image data nodes.
   */
  void SetImageNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes);

  /**
   * \brief Sets the mask nodes whose statistics should be displayed.
   * \param[in] nodes Vector of mask data nodes.
   */
  void SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes);

  /**
   * \brief Clears all data from the model and disables the widget controls.
   */
  void Reset();

  /**
   * \brief Sets whether zero-valued voxels should be ignored when selecting statistics.
   * \param[in] _arg True to ignore zero-valued voxels; false to include them.
   */
  void SetIgnoreZeroValueVoxel(bool _arg);

  /**
   * \brief Returns whether zero-valued voxels are currently being ignored.
   * \return True if zero-valued voxels are ignored; false otherwise.
   */
  bool GetIgnoreZeroValueVoxel() const;

  /**
   * \brief Sets the number of histogram bins used to select matching statistics.
   * \param[in] nbins The number of histogram bins.
   */
  void SetHistogramNBins(unsigned int nbins);

  /**
   * \brief Returns the current number of histogram bins.
   * \return The number of histogram bins.
   */
  unsigned int GetHistogramNBins() const;

  /**
   * \brief Offers or hides check boxes on the label rows of the statistics tree.
   * \sa QmitkImageStatisticsTreeModel::SetLabelsCheckable
   */
  void SetLabelsCheckable(bool checkable);

  /**
   * \brief Returns whether the given label is checked in the statistics tree.
   * \sa QmitkImageStatisticsTreeModel::IsLabelChecked
   */
  bool IsLabelChecked(mitk::ImageStatisticsContainer::LabelValueType labelValue) const;

signals:
  /**
   * \brief Emitted when the user toggles the "ignore zero-valued voxels" checkbox.
   * \param[in] status The new checkbox state.
   */
  void IgnoreZeroValuedVoxelStateChanged(Qt::CheckState status);

  /** \brief Emitted when the user changed the check state of a label row. */
  void LabelCheckStateChanged();

  /**
   * \brief Emitted after the tree was rebuilt because an input was renamed or a label was
   * renamed or recolored. The statistics themselves are unchanged.
   */
  void InputDisplayChanged();

private:
  void CreateConnections();
  void OnDataAvailable();

  /** \brief  Saves the image statistics to the clipboard */
  void OnClipboardButtonClicked();

  std::unique_ptr<Ui::QmitkImageStatisticsControls> m_Controls;
  QmitkImageStatisticsTreeModel *m_imageStatisticsModel;
  QSortFilterProxyModel *m_ProxyModel;
};
#endif
