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
#include <set>
#include <string>

class QMenu;
class QmitkImageStatisticsTreeModel;

namespace Ui
{
  class QmitkImageStatisticsControls;
}

/**
 * \brief Widget for displaying image statistics in a tree view with clipboard export.
 *
 * This widget wraps a QmitkImageStatisticsTreeModel in a tree view and provides controls for
 * ignoring zero-valued voxels, choosing the shown statistics, and copying the shown
 * statistics to the clipboard. It automatically enables its controls when statistics data
 * becomes available.
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

  /**
   * \brief Hides the columns of the given statistics in the table and in the clipboard export.
   *
   * Keys that none of the current statistics has are kept, so the choice also holds for
   * inputs that are selected later.
   *
   * \param[in] keys Statistic keys as used by mitk::ImageStatisticsConstants.
   */
  void SetHiddenStatistics(const std::set<std::string>& keys);

  /** \brief Returns the keys of the hidden statistics. \sa SetHiddenStatistics */
  std::set<std::string> GetHiddenStatistics() const;

signals:
  /**
   * \brief Emitted when the user toggles the "Ignore 0-value voxels" checkbox.
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

  /** \brief Emitted when the user changed which statistics are hidden. */
  void HiddenStatisticsChanged();

private:
  class StatisticsFilterProxyModel;

  void CreateConnections();
  void OnDataAvailable();

  /** \brief  Saves the image statistics to the clipboard */
  void OnClipboardButtonClicked();

  /** Fills the menu with the statistics to show or hide. If tableColumn denotes a statistic
  column of the table, the menu starts with actions that act on just this statistic. */
  void PopulateStatisticsMenu(QMenu* menu, int tableColumn = 0);

  /** Hides the given statistics, updates the dependent controls and notifies listeners. */
  void ChangeHiddenStatistics(const std::set<std::string>& keys);

  void ResizeColumnsToContents();
  void UpdateStatisticsFilterButton();

  std::unique_ptr<Ui::QmitkImageStatisticsControls> m_Controls;
  QmitkImageStatisticsTreeModel *m_imageStatisticsModel;
  StatisticsFilterProxyModel *m_ProxyModel;
};
#endif
