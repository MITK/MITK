/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkImageStatisticsTreeModel.h>

#include "QmitkImageStatisticsTreeItem.h"
#include <mitkImageStatisticsConstants.h>
#include <mitkImageStatisticsContainerManager.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkMultiLabelEvents.h>

#include <QmitkIconTheme.h>

#include <QLocale>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <map>
#include <variant>

namespace
{
  /** Header text and header tooltip of a statistic. An empty tooltip means that the header
  text already says everything there is to say. */
  struct StatisticDisplay
  {
    QString name;
    QString toolTip;
  };

  /** Maps the keys of mitk::ImageStatisticsConstants to what the header shows. The keys are
  identifiers, hence they stay technical, while the header spells things out. Statistics
  without an entry, e.g. contributed by other modules, are shown by their key. */
  const std::map<std::string, StatisticDisplay>& GetStatisticDisplays()
  {
    using Constants = mitk::ImageStatisticsConstants;

    static const std::map<std::string, StatisticDisplay> displays = {
      { Constants::MEAN(), { QStringLiteral("Mean"), QStringLiteral("Arithmetic mean") } },
      { Constants::MEDIAN(), { QStringLiteral("Median"), QStringLiteral("Middle value of the distribution") } },
      { Constants::STANDARDDEVIATION(), { QStringLiteral("Std. dev."), QStringLiteral("Standard deviation") } },
      { Constants::VARIANCE(), { QStringLiteral("Variance"), QStringLiteral("Squared standard deviation") } },
      { Constants::RMS(), { QStringLiteral("RMS"), QStringLiteral("Root mean square") } },
      { Constants::MAXIMUM(), { QStringLiteral("Max"), QStringLiteral("Maximum intensity") } },
      { Constants::MAXIMUMPOSITION(), { QStringLiteral("Max position"), QStringLiteral("Voxel index of the maximum intensity") } },
      { Constants::MINIMUM(), { QStringLiteral("Min"), QStringLiteral("Minimum intensity") } },
      { Constants::MINIMUMPOSITION(), { QStringLiteral("Min position"), QStringLiteral("Voxel index of the minimum intensity") } },
      { Constants::NUMBEROFVOXELS(), { QStringLiteral("Voxels"), QStringLiteral("Number of voxels") } },
      // The superscript is written as its UTF-8 bytes to keep the source file plain ASCII.
      { Constants::VOLUME(), { QStringLiteral("Volume [mm\xC2\xB3]"), QStringLiteral("Volume in cubic millimeters") } },
      { Constants::SKEWNESS(), { QStringLiteral("Skewness"), QStringLiteral("Asymmetry of the intensity distribution") } },
      { Constants::KURTOSIS(), { QStringLiteral("Kurtosis"), QStringLiteral("Tailedness of the intensity distribution") } },
      { Constants::UNIFORMITY(), { QStringLiteral("Uniformity"), QStringLiteral("Sum of the squared histogram probabilities") } },
      { Constants::ENTROPY(), { QStringLiteral("Entropy"), QStringLiteral("Shannon entropy of the histogram") } },
      { Constants::MPP(), { QStringLiteral("MPP"), QStringLiteral("Mean of positive pixels") } },
      { Constants::UPP(), { QStringLiteral("UPP"), QStringLiteral("Uniformity of positive pixels") } }
    };

    return displays;
  }

  QString GetStatisticName(const std::string& key)
  {
    const auto& displays = GetStatisticDisplays();
    const auto finding = displays.find(key);

    return displays.cend() != finding
      ? finding->second.name
      : QString::fromStdString(key);
  }

  QString GetStatisticToolTip(const std::string& key)
  {
    const auto& displays = GetStatisticDisplays();
    const auto finding = displays.find(key);

    return displays.cend() != finding
      ? finding->second.toolTip
      : QString();
  }

  /** Significant digits the largest value of a column should show, and the range the
  resulting number of decimal places is kept in. */
  constexpr int SIGNIFICANT_DIGITS = 5;
  constexpr int MIN_DECIMALS = 2;
  constexpr int MAX_DECIMALS = 6;

  int GetDecimals(double maxAbsValue)
  {
    const int magnitude = maxAbsValue > 0.0
      ? static_cast<int>(std::floor(std::log10(maxAbsValue)))
      : 0;

    return std::clamp(SIGNIFICANT_DIGITS - 1 - magnitude, MIN_DECIMALS, MAX_DECIMALS);
  }

  /** Decimal places are chosen per column instead of per value: all values of a column then
  share the position of the decimal point, while columns of very different magnitude, e.g.
  volume and uniformity, still show a comparable number of significant digits. */
  std::vector<int> ComputeColumnDecimals(const std::vector<mitk::ImageStatisticsContainer::ConstPointer>& statistics,
    const std::vector<std::string>& statisticNames)
  {
    std::vector<double> maxAbsValues(statisticNames.size(), 0.0);

    for (const auto& container : statistics)
    {
      for (const auto labelValue : container->GetExistingLabelValues())
      {
        for (mitk::TimeStepType timeStep = 0; timeStep < container->GetTimeSteps(); ++timeStep)
        {
          if (!container->StatisticsExist(labelValue, timeStep))
            continue;

          const auto& statisticsObject = container->GetStatistics(labelValue, timeStep);

          for (size_t i = 0; i < statisticNames.size(); ++i)
          {
            if (!statisticsObject.HasStatistic(statisticNames[i]))
              continue;

            const auto value = statisticsObject.GetValueNonConverted(statisticNames[i]);
            const auto* realValue = std::get_if<mitk::ImageStatisticsContainer::RealType>(&value);

            // A non-finite value, e.g. the skewness of a single voxel, must not set the format.
            if (nullptr != realValue && std::isfinite(*realValue))
              maxAbsValues[i] = std::max(maxAbsValues[i], std::abs(*realValue));
          }
        }
      }
    }

    std::vector<int> decimals;
    decimals.reserve(maxAbsValues.size());

    std::transform(maxAbsValues.cbegin(), maxAbsValues.cend(), std::back_inserter(decimals), GetDecimals);

    return decimals;
  }

  /** Formats a statistic value for display in the locale of the user. Anything but a number,
  e.g. a voxel index or a placeholder of a pending calculation, is passed through. */
  QVariant FormatValue(const QVariant& value, int decimals)
  {
    switch (value.typeId())
    {
      case QMetaType::Double:
        return QLocale().toString(value.toDouble(), 'f', decimals);

      case QMetaType::ULong:
      case QMetaType::ULongLong:
        return QLocale().toString(value.toULongLong());

      default:
        return value;
    }
  }

  /** Observes the "name" property of the passed node, if it has one. The property object
  itself has to be observed because DataNode::SetName() writes in place into a "name"
  property owned by the BaseData if there is one (e.g. for DICOM images), which modifies
  neither the node nor the data. DataNode::GetProperty() resolves both possible owners and
  is what DataNode::GetName() reads. */
  void AddNameObserver(const mitk::DataNode* node,
    std::vector<mitk::ITKEventObserverGuard>& observers,
    const std::function<void(const itk::EventObject&)>& handler)
  {
    if (nullptr == node)
      return;

    const auto* nameProperty = node->GetProperty("name");

    if (nullptr != nameProperty)
      observers.emplace_back(nameProperty, itk::ModifiedEvent(), handler);
  }

  /** Appends the label values of all label rows below item in depth-first (tree) order. */
  void CollectLabelValues(QmitkImageStatisticsTreeItem* item, std::vector<mitk::ImageStatisticsContainer::LabelValueType>& values)
  {
    if (const auto label = item->GetLabelInstance(); label.IsNotNull())
      values.push_back(label->GetValue());

    for (int row = 0; row < item->childCount(); ++row)
      CollectLabelValues(item->child(row), values);
  }
}

QmitkImageStatisticsTreeModel::QmitkImageStatisticsTreeModel(QObject *parent) : QmitkAbstractDataStorageModel(parent)
{
  m_RootItem = std::make_unique<QmitkImageStatisticsTreeItem>();
  m_WIPIcon = QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/hourglass-half-solid.svg"));
}

QmitkImageStatisticsTreeModel ::~QmitkImageStatisticsTreeModel()
{
  // set data storage to nullptr so that the event listener gets removed
  this->SetDataStorage(nullptr);
};

void QmitkImageStatisticsTreeModel::DataStorageChanged()
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::NodePredicateChanged()
{
  emit beginResetModel();
  UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

int QmitkImageStatisticsTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  int columns = m_StatisticNames.size() + 1;
  return columns;
}

int QmitkImageStatisticsTreeModel::rowCount(const QModelIndex &parent) const
{
  QmitkImageStatisticsTreeItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = m_RootItem.get();
  else
    parentItem = static_cast<QmitkImageStatisticsTreeItem *>(parent.internalPointer());

  return parentItem->childCount();
}

QVariant QmitkImageStatisticsTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  QmitkImageStatisticsTreeItem* item = static_cast<QmitkImageStatisticsTreeItem*>(index.internalPointer());

  if (role == Qt::DisplayRole)
  {
    const auto column = index.column();

    if (column > 0 && static_cast<size_t>(column - 1) < m_ColumnDecimals.size())
      return FormatValue(item->data(column), m_ColumnDecimals[column - 1]);

    return item->data(column);
  }
  else if (role == Qt::EditRole)
  {
    // The unformatted value, e.g. for the clipboard export.
    return item->data(index.column());
  }
  else if (role == Qt::TextAlignmentRole && index.column() > 0)
  {
    return QVariant(static_cast<int>(Qt::AlignRight | Qt::AlignVCenter));
  }
  else if (role == Qt::DecorationRole && index.column() == 0)
  {
    if (item->isWIP() && item->childCount() == 0)
      return QVariant(m_WIPIcon);
    else if (!item->isWIP())
    {
      auto label = item->GetLabelInstance();
      if (label.IsNotNull())
      {
        QPixmap pixmap(QSize(20,20));
        QColor color(label->GetColor().GetRed() * 255, label->GetColor().GetGreen() * 255, label->GetColor().GetBlue() * 255);
        pixmap.fill(color);

        return QVariant(QIcon(pixmap));
      }
    }
  }
  else if (role == Qt::CheckStateRole && this->IsCheckable(index))
  {
    const bool checked = this->IsLabelChecked(item->GetLabelInstance()->GetValue());
    return static_cast<int>(checked ? Qt::Checked : Qt::Unchecked);
  }
  else if (role == Qt::ToolTipRole && this->IsCheckable(index))
  {
    return QStringLiteral("Show the histogram of this label");
  }
  return QVariant();
}

bool QmitkImageStatisticsTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (role != Qt::CheckStateRole || !this->IsCheckable(index) || !m_CheckedLabelValues.has_value())
    return false;

  const auto* item = static_cast<const QmitkImageStatisticsTreeItem*>(index.internalPointer());
  const auto labelValue = item->GetLabelInstance()->GetValue();
  const bool checked = value.toInt() == Qt::Checked;

  if (checked == this->IsLabelChecked(labelValue))
    return false;

  if (checked)
    m_CheckedLabelValues->insert(labelValue);
  else
    m_CheckedLabelValues->erase(labelValue);

  emit dataChanged(index, index, { Qt::CheckStateRole });
  emit labelCheckStateChanged();

  return true;
}

bool QmitkImageStatisticsTreeModel::IsLabelChecked(mitk::ImageStatisticsContainer::LabelValueType labelValue) const
{
  return m_LabelRowValues.size() <= 1 || !m_CheckedLabelValues.has_value() || m_CheckedLabelValues->contains(labelValue);
}

void QmitkImageStatisticsTreeModel::ReconcileCheckedLabels()
{
  if (m_LabelRowValues.empty())
    return;

  if (m_CheckedLabelValues.has_value())
  {
    std::erase_if(*m_CheckedLabelValues, [this](const auto labelValue)
    {
      return std::find(m_LabelRowValues.begin(), m_LabelRowValues.end(), labelValue) == m_LabelRowValues.end();
    });
  }

  if (!m_CheckedLabelValues.has_value() || m_CheckedLabelValues->empty())
    m_CheckedLabelValues.emplace().insert(m_LabelRowValues.front());
}

void QmitkImageStatisticsTreeModel::SetLabelsCheckable(bool checkable)
{
  if (m_LabelsCheckable == checkable)
    return;

  // A reset rather than dataChanged() per label row: the rows live at varying depths
  // (group rows are optional), and every other setter of this model resets as well.
  emit beginResetModel();
  m_LabelsCheckable = checkable;
  emit endResetModel();
  emit modelChanged();
}

bool QmitkImageStatisticsTreeModel::IsCheckable(const QModelIndex& index) const
{
  if (!m_LabelsCheckable || m_LabelRowValues.size() <= 1 || !index.isValid() || index.column() != 0)
    return false;

  const auto* item = static_cast<const QmitkImageStatisticsTreeItem*>(index.internalPointer());
  return item->GetLabelInstance().IsNotNull();
}

QModelIndex QmitkImageStatisticsTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  QmitkImageStatisticsTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = m_RootItem.get();
  else
    parentItem = static_cast<QmitkImageStatisticsTreeItem *>(parent.internalPointer());

  QmitkImageStatisticsTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex QmitkImageStatisticsTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  QmitkImageStatisticsTreeItem *childItem = static_cast<QmitkImageStatisticsTreeItem *>(child.internalPointer());
  QmitkImageStatisticsTreeItem *parentItem = childItem->parentItem();

  if (parentItem == m_RootItem.get())
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags QmitkImageStatisticsTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return {};

  auto flags = QAbstractItemModel::flags(index);

  if (this->IsCheckable(index))
    flags |= Qt::ItemIsUserCheckable;

  return flags;
}

QVariant QmitkImageStatisticsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::Horizontal != orientation)
    return QVariant();

  if (Qt::DisplayRole == role)
  {
    return section == 0
      ? QVariant(QStringLiteral("Images / Masks"))
      : QVariant(GetStatisticName(m_StatisticNames.at(section - 1)));
  }
  else if (Qt::ToolTipRole == role)
  {
    if (section == 0)
      return QVariant(QStringLiteral("Images and their masks, broken down into groups, labels and time steps"));

    const auto toolTip = GetStatisticToolTip(m_StatisticNames.at(section - 1));

    if (!toolTip.isEmpty())
      return QVariant(toolTip);
  }
  else if (Qt::TextAlignmentRole == role)
  {
    return QVariant(static_cast<int>(Qt::AlignCenter));
  }

  return QVariant();
}

void QmitkImageStatisticsTreeModel::SetImageNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes)
{
  std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int>> tempNodes;
  for (const auto &node : nodes)
  {
    auto data = node->GetData();
    if (data)
    {
      auto timeSteps = data->GetTimeSteps();
      for (unsigned int i = 0; i < timeSteps; i++)
      {
        tempNodes.push_back(std::make_pair(node, i));
      }
    }
  }

  emit beginResetModel();
  m_TimeStepResolvedImageNodes = std::move(tempNodes);
  m_ImageNodes = nodes;
  m_CheckedLabelValues.reset();
  this->UpdateInputObservers();
  this->UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes)
{
  std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int>> tempNodes;
  for (const auto &node : nodes)
  {
    auto data = node->GetData();
    if (data)
    {
      auto timeSteps = data->GetTimeSteps();
      // special case: apply one mask to each time step of an 4D image
      if (timeSteps == 1 && m_TimeStepResolvedImageNodes.size() > 1)
      {
        timeSteps = m_TimeStepResolvedImageNodes.size();
      }
      for (unsigned int i = 0; i < timeSteps; i++)
      {
        tempNodes.push_back(std::make_pair(node, i));
      }
    }
  }

  emit beginResetModel();
  m_TimeStepResolvedMaskNodes = std::move(tempNodes);
  m_MaskNodes = nodes;
  m_CheckedLabelValues.reset();
  this->UpdateInputObservers();
  this->UpdateByDataStorage();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::Clear()
{
  emit beginResetModel();
  m_InputObservers.clear();
  m_Statistics.clear();
  m_ImageNodes.clear();
  m_TimeStepResolvedImageNodes.clear();
  m_MaskNodes.clear();
  m_TimeStepResolvedMaskNodes.clear();
  m_StatisticNames.clear();
  m_ColumnDecimals.clear();
  m_CheckedLabelValues.reset();
  emit endResetModel();
  emit modelChanged();
}

void QmitkImageStatisticsTreeModel::SetIgnoreZeroValueVoxel(bool _arg)
{
  if (m_IgnoreZeroValueVoxel != _arg)
  {
    emit beginResetModel();
    m_IgnoreZeroValueVoxel = _arg;
    UpdateByDataStorage();
    emit endResetModel();
    emit modelChanged();
  }
}

bool QmitkImageStatisticsTreeModel::GetIgnoreZeroValueVoxel() const
{
  return this->m_IgnoreZeroValueVoxel;
}

void QmitkImageStatisticsTreeModel::SetHistogramNBins(unsigned int nbins)
{
  if (m_HistogramNBins != nbins)
  {
    emit beginResetModel();
    m_HistogramNBins = nbins;
    UpdateByDataStorage();
    emit endResetModel();
    emit modelChanged();
  }
}

unsigned int QmitkImageStatisticsTreeModel::GetHistogramNBins() const
{
  return this->m_HistogramNBins;
}

void QmitkImageStatisticsTreeModel::UpdateInputObservers()
{
  m_InputObservers.clear();

  std::function<void(const itk::EventObject&)> handler =
    [this](const itk::EventObject&) { this->RequestModelUpdate(); };

  for (const auto& node : m_ImageNodes)
    AddNameObserver(node, m_InputObservers, handler);

  for (const auto& node : m_MaskNodes)
  {
    AddNameObserver(node, m_InputObservers, handler);

    // Renaming or recoloring a label or renaming a group modifies only the segmentation,
    // never its node, therefore the segmentation has to be observed directly.
    const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(
      node.IsNull() ? nullptr : node->GetData());

    if (nullptr != segmentation)
    {
      m_InputObservers.emplace_back(segmentation, mitk::LabelModifiedEvent(), handler);
      m_InputObservers.emplace_back(segmentation, mitk::GroupModifiedEvent(), handler);
    }
  }
}

void QmitkImageStatisticsTreeModel::RequestModelUpdate()
{
  // Atomic, because a label can also be modified by a worker thread, e.g. by a
  // segmentation algorithm that names its results.
  if (m_ModelUpdatePending.exchange(true))
    return;

  // Deferred on purpose: bulk operations on a segmentation (MultiLabelSegmentation::
  // ApplyToLabels, e.g. behind "show all labels") send one LabelModifiedEvent per label.
  // Coalescing them avoids one complete model rebuild per event. It also keeps the reset
  // out of the event invocation of the sender.
  QMetaObject::invokeMethod(this, [this]()
    {
      m_ModelUpdatePending = false;

      emit beginResetModel();
      {
        // Deliberately no UpdateByDataStorage(): a renamed node or label does not change
        // which statistics apply, but modifying a label bumps the modification time of the
        // segmentation, which would make ImageStatisticsContainerManager discard the still
        // valid statistics as outdated.
        std::lock_guard<std::mutex> locked(m_Mutex);
        this->BuildHierarchicalModel();
      }
      emit endResetModel();
      emit modelChanged();
      emit inputDisplayChanged();
    }, Qt::QueuedConnection);
}

void QmitkImageStatisticsTreeModel::UpdateByDataStorage()
{
  StatisticsContainerVector newStatistics;

  auto datamanager = m_DataStorage.Lock();

  if (datamanager.IsNotNull())
  {
    for (const auto &image : m_ImageNodes)
    {
      if (m_MaskNodes.empty())
      {
        auto stats = mitk::ImageStatisticsContainerManager::GetImageStatistics(datamanager, image->GetData(), nullptr, m_IgnoreZeroValueVoxel, m_HistogramNBins, true, false);

        if (stats.IsNotNull())
        {
          newStatistics.emplace_back(stats);
        }
      }
      else
      {
        for (const auto &mask : m_MaskNodes)
        {
          auto stats =
            mitk::ImageStatisticsContainerManager::GetImageStatistics(datamanager, image->GetData(), mask->GetData(), m_IgnoreZeroValueVoxel, m_HistogramNBins, true, false);
          if (stats.IsNotNull())
          {
            newStatistics.emplace_back(stats);
          }
        }
      }
    }
    if (!newStatistics.empty())
    {
      emit dataAvailable();
    }
  }

  {
    std::lock_guard<std::mutex> locked(m_Mutex);
    m_Statistics = newStatistics;

    m_StatisticNames = mitk::GetAllStatisticNames(m_Statistics);
    BuildHierarchicalModel();
    m_BuildTime.Modified();
  }
}

void AddTimeStepTreeItems(const mitk::ImageStatisticsContainer* statistic, const mitk::DataNode* imageNode, const mitk::DataNode* maskNode, mitk::ImageStatisticsContainer::LabelValueType labelValue, const std::vector<std::string>& statisticNames, bool isWIP, QmitkImageStatisticsTreeItem* parentItem)
{
  // 4. hierarchy level: time steps (optional, only if >1 time step)
  if (statistic->GetTimeSteps() > 1)
  {
    for (unsigned int i = 0; i < statistic->GetTimeSteps(); i++)
    {
      QString timeStepLabel = "[" + QString::number(i) + "] " +
        QString::number(statistic->GetTimeGeometry()->TimeStepToTimePoint(i)) + " ms";
      if (statistic->StatisticsExist(labelValue, i))
      {
        auto statisticsItem = new QmitkImageStatisticsTreeItem(
          statistic->GetStatistics(labelValue,i), statisticNames, timeStepLabel, isWIP, parentItem, imageNode, maskNode);
        parentItem->appendChild(statisticsItem);
      }
      else
      {
        auto statisticsItem = new QmitkImageStatisticsTreeItem(statisticNames, timeStepLabel, isWIP, true, parentItem, imageNode, maskNode);
        parentItem->appendChild(statisticsItem);
      }
    }
  }
}

void AddLabelTreeItems(const mitk::ImageStatisticsContainer* statistic, const mitk::DataNode* imageNode, const mitk::DataNode* maskNode, mitk::ImageStatisticsContainer::LabelValueVectorType labelValues, const std::vector<std::string>& statisticNames, bool isWIP, QmitkImageStatisticsTreeItem* parentItem)
{
  // 3. hierarchy level: labels (optional, only if labels >1)
  for (const auto labelValue : labelValues)
  {
    if (labelValue != mitk::ImageStatisticsContainer::NO_MASK_LABEL_VALUE)
    {
      //currently we only show statistics of the labeled pixel if a mask is provided
      QString labelLabel = QStringLiteral("unnamed label");
      const auto multiLabelSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(maskNode->GetData());
      mitk::Label::ConstPointer labelInstance;
      if (nullptr != multiLabelSeg)
      {
        labelInstance = multiLabelSeg->GetLabel(labelValue);
        if (labelInstance.IsNotNull())
        {
          labelLabel = QString::fromStdString(mitk::LabelSetImageHelper::CreateDisplayLabelName(multiLabelSeg, labelInstance));
        }
        else
        {
          labelLabel = QString("Unknown label ID ") + QString::number(labelValue);
        }
      }
      QmitkImageStatisticsTreeItem* labelItem = nullptr;

      if (labelInstance.IsNotNull())
      {
        if (statistic->GetTimeSteps() == 1)
        {
          // add statistical values directly in this hierarchy level
          auto statisticsObject = statistic->GetStatistics(labelValue, 0);
          labelItem = new QmitkImageStatisticsTreeItem(statisticsObject, statisticNames, labelLabel, isWIP, parentItem, imageNode, maskNode, labelInstance);
        }
        else
        {
          labelItem = new QmitkImageStatisticsTreeItem(statisticNames, labelLabel, isWIP, false, parentItem, imageNode, maskNode, labelInstance);
          AddTimeStepTreeItems(statistic, imageNode, maskNode, labelValue, statisticNames, isWIP, labelItem);
        }
      }
      else
      {
        labelItem = new QmitkImageStatisticsTreeItem(statisticNames, labelLabel, isWIP, true, parentItem, imageNode, maskNode);
      }

      parentItem->appendChild(labelItem);
    }
  }
}

/** Adds the label rows of a mask, ordered like the Segmentation View if the mask is a
segmentation: grouped by group, within a group by class name and then by label value.
The value order comes for free because the statistics container enumerates its label
values sorted, and SplitLabelValuesByClassName keeps that order inside a class. Group rows
are only added if the segmentation has more than one group, so the common single-group
case keeps its compact tree. */
void AddLabelTreeItemsForMask(const mitk::ImageStatisticsContainer* statistic, const mitk::DataNode* imageNode, const mitk::DataNode* maskNode, const mitk::ImageStatisticsContainer::LabelValueVectorType& labelValues, const std::vector<std::string>& statisticNames, bool isWIP, QmitkImageStatisticsTreeItem* parentItem)
{
  const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(maskNode->GetData());

  if (nullptr == segmentation)
  {
    AddLabelTreeItems(statistic, imageNode, maskNode, labelValues, statisticNames, isWIP, parentItem);
    return;
  }

  // Statistics can outlive a label (e.g. after a label was removed and the statistics were
  // not recomputed yet), so only values the segmentation still knows can be grouped.
  mitk::ImageStatisticsContainer::LabelValueVectorType knownValues;
  mitk::ImageStatisticsContainer::LabelValueVectorType unknownValues;

  for (const auto labelValue : labelValues)
    (segmentation->ExistLabel(labelValue) ? knownValues : unknownValues).push_back(labelValue);

  const bool showGroups = segmentation->GetNumberOfGroups() > 1;

  for (mitk::MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < segmentation->GetNumberOfGroups(); ++groupID)
  {
    mitk::ImageStatisticsContainer::LabelValueVectorType groupValues;

    for (const auto& [className, classValues] : mitk::LabelSetImageHelper::SplitLabelValuesByClassName(segmentation, groupID, knownValues))
      groupValues.insert(groupValues.end(), classValues.begin(), classValues.end());

    if (groupValues.empty())
      continue;

    auto groupParentItem = parentItem;

    if (showGroups)
    {
      const auto groupLabel = QString::fromStdString(mitk::LabelSetImageHelper::CreateDisplayGroupName(segmentation, groupID));
      groupParentItem = new QmitkImageStatisticsTreeItem(statisticNames, groupLabel, isWIP, false, parentItem, imageNode, maskNode);
      parentItem->appendChild(groupParentItem);
    }

    AddLabelTreeItems(statistic, imageNode, maskNode, groupValues, statisticNames, isWIP, groupParentItem);
  }

  AddLabelTreeItems(statistic, imageNode, maskNode, unknownValues, statisticNames, isWIP, parentItem);
}

void QmitkImageStatisticsTreeModel::BuildHierarchicalModel()
{
  // reset old model
  m_RootItem.reset(new QmitkImageStatisticsTreeItem());

  m_ColumnDecimals = ComputeColumnDecimals(m_Statistics, m_StatisticNames);

  std::map<mitk::DataNode::ConstPointer, QmitkImageStatisticsTreeItem *> dataNodeToTreeItem;

  for (const auto &statistic : m_Statistics)
  {
    bool isWIP = statistic->IsWIP();
    // get the connected image data node/mask data node
    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    auto imageOfStatisticsPredicate = imageRule->GetDestinationsDetector(statistic);
    auto imageFinding = std::find_if(m_ImageNodes.begin(), m_ImageNodes.end(), [&imageOfStatisticsPredicate](const mitk::DataNode::ConstPointer& testNode) { return imageOfStatisticsPredicate->CheckNode(testNode); });

    auto maskRule = mitk::StatisticsToMaskRelationRule::New();
    auto maskOfStatisticsPredicate = maskRule->GetDestinationsDetector(statistic);
    auto maskFinding = std::find_if(m_MaskNodes.begin(), m_MaskNodes.end(), [&maskOfStatisticsPredicate](const mitk::DataNode::ConstPointer& testNode) { return maskOfStatisticsPredicate->CheckNode(testNode); });

    if (imageFinding == m_ImageNodes.end())
    {
      mitkThrow() << "no image found connected to statistic" << statistic << " Aborting.";
    }

    auto& image = *imageFinding;

    // image: 1. hierarchy level
    QmitkImageStatisticsTreeItem *imageItem = nullptr;
    auto search = dataNodeToTreeItem.find(image);
    if (search != dataNodeToTreeItem.end())
    {
      // the tree item was created previously
      imageItem = search->second;
    }
    else
    {
      QString imageLabel = QString::fromStdString(image->GetName());
      if (statistic->GetTimeSteps() == 1 && maskFinding == m_MaskNodes.end())
      {
        auto labelValue = isWIP ? mitk::ImageStatisticsContainer::NO_MASK_LABEL_VALUE : statistic->GetExistingLabelValues().front();

        auto statisticsObject = isWIP ? mitk::ImageStatisticsContainer::ImageStatisticsObject() : statistic->GetStatistics(labelValue, 0);
        // create the final statistics tree item
        imageItem = new QmitkImageStatisticsTreeItem(statisticsObject, m_StatisticNames, imageLabel, isWIP, m_RootItem.get(), image);
      }
      else
      {
        imageItem = new QmitkImageStatisticsTreeItem(m_StatisticNames, imageLabel, isWIP, false, m_RootItem.get(), image);
      }
      m_RootItem->appendChild(imageItem);
      dataNodeToTreeItem.emplace(image, imageItem);
    }

    if (maskFinding != m_MaskNodes.end())
    {
      const auto labelValues = statistic->GetExistingLabelValues(); //currently we not support showing the statistics for unlabeled pixels if a mask exist

      // mask: 2. hierarchy level exists
      auto& mask = *maskFinding;
      QString maskLabel = QString::fromStdString(mask->GetName());
      QmitkImageStatisticsTreeItem* maskItem;

      // A segmentation always gets one row per label so that the name and color of a
      // label are shown even if it is the only one left. Other masks with a single label
      // collapse into the mask row.
      const bool showLabelRows = labelValues.size() > 1
        || nullptr != dynamic_cast<const mitk::MultiLabelSegmentation*>(mask->GetData());

      if (labelValues.empty())
      {
        //all labels are empty -> no stats are computed
        maskItem = new QmitkImageStatisticsTreeItem(m_StatisticNames, maskLabel, isWIP, true, imageItem, image, mask);
      }
      else if (statistic->GetTimeSteps() == 1 && !showLabelRows)
      {
        // add statistical values directly in this hierarchy level
        auto statisticsObject = isWIP ? mitk::ImageStatisticsContainer::ImageStatisticsObject() : statistic->GetStatistics(labelValues.front(), 0);
        maskItem = new QmitkImageStatisticsTreeItem(statisticsObject, m_StatisticNames, maskLabel, isWIP, imageItem, image, mask);
      }
      else
      {
        maskItem = new QmitkImageStatisticsTreeItem(m_StatisticNames, maskLabel, isWIP, false, imageItem, image, mask);

        if (showLabelRows)
        {
          AddLabelTreeItemsForMask(statistic, image, mask, labelValues, m_StatisticNames, isWIP, maskItem);
        }
        else
        {
          AddTimeStepTreeItems(statistic, image, mask, labelValues.front(), m_StatisticNames, isWIP, maskItem);
        }
      }

      imageItem->appendChild(maskItem);
    }
    else
    {
      //no mask -> but multi time step
      auto labelValue = isWIP ? mitk::ImageStatisticsContainer::NO_MASK_LABEL_VALUE : statistic->GetExistingLabelValues().front();

      AddTimeStepTreeItems(statistic, image, nullptr, labelValue, m_StatisticNames, isWIP, imageItem);
    }
  }

  m_LabelRowValues.clear();
  CollectLabelValues(m_RootItem.get(), m_LabelRowValues);
  this->ReconcileCheckedLabels();
}

void QmitkImageStatisticsTreeModel::NodeRemoved(const mitk::DataNode* changedNode)
{
  bool isRelevantNode = (nullptr != dynamic_cast<const mitk::ImageStatisticsContainer*>(changedNode->GetData()));

  if (isRelevantNode)
  {
    emit beginResetModel();
    UpdateByDataStorage();
    emit endResetModel();
    emit modelChanged();
  }
}

void QmitkImageStatisticsTreeModel::NodeAdded(const mitk::DataNode * changedNode)
{
  bool isRelevantNode = (nullptr != dynamic_cast<const mitk::ImageStatisticsContainer*>(changedNode->GetData()));

  if (isRelevantNode)
  {
      emit beginResetModel();
      UpdateByDataStorage();
      emit endResetModel();
      emit modelChanged();
  }
}

void QmitkImageStatisticsTreeModel::NodeChanged(const mitk::DataNode * changedNode)
{
  bool isInputNode = m_ImageNodes.end() != std::find(m_ImageNodes.begin(), m_ImageNodes.end(), changedNode);
  isInputNode = isInputNode || (m_MaskNodes.end() != std::find(m_MaskNodes.begin(), m_MaskNodes.end(), changedNode));

  if (isInputNode)
  {
    // The "name" property object can be created late or be replaced as a whole (e.g. by
    // DataNode::SetData() clearing the property list), so rebind instead of letting the
    // observer go stale unnoticed.
    this->UpdateInputObservers();
  }

  const auto* data = changedNode->GetData();
  const bool isRelevantNode = isInputNode || (nullptr != dynamic_cast<const mitk::ImageStatisticsContainer*>(data));

  if (isRelevantNode && nullptr != data && m_BuildTime.GetMTime() < data->GetMTime())
  {
    emit beginResetModel();
    this->UpdateByDataStorage();
    emit endResetModel();
    emit modelChanged();
  }
}
