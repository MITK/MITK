/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkImageStatisticsWidget.h>
#include <ui_QmitkImageStatisticsWidget.h>

#include <QmitkStatisticsModelToStringConverter.h>
#include <QmitkImageStatisticsTreeModel.h>

#include <mitkImageStatisticsContainer.h>

#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QStyledItemDelegate>

namespace
{
  /** Gap between the right aligned value of a column and the value of the column before,
  which would otherwise nearly touch. */
  constexpr int COLUMN_GAP_IN_CHARACTERS = 2;

  /** Tints every other column in the color the style reserves for alternating rows, which
  makes it easier to follow a single statistic down the table. The color is taken from the
  palette of the view, so that it also holds after a theme switch. The tint starts at the
  first statistic column: a cell of the tree column starts at the indentation of its row,
  hence a tint would leave a ragged edge there. */
  class AlternatingColumnDelegate : public QStyledItemDelegate
  {
  public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      auto size = QStyledItemDelegate::sizeHint(option, index);

      if (index.column() > 0)
        size.rwidth() += COLUMN_GAP_IN_CHARACTERS * option.fontMetrics.averageCharWidth();

      return size;
    }

  protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
      QStyledItemDelegate::initStyleOption(option, index);

      if (index.column() % 2 == 1)
        option->backgroundBrush = option->palette.brush(QPalette::AlternateBase);
    }
  };

  /** Keeps the menu open when a check box entry is clicked or toggled with the space key, so
  that several statistics can be shown or hidden in one go. Enter still toggles and closes. */
  class StayOpenMenu : public QMenu
  {
  public:
    using QMenu::QMenu;

  protected:
    void keyPressEvent(QKeyEvent* event) override
    {
      if (Qt::Key_Space == event->key() && !event->isAutoRepeat() && TriggerIfCheckable(this->activeAction()))
        return;

      QMenu::keyPressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
      auto* action = this->activeAction();

      // The highlighted entry must also be the one under the cursor: a click on a scroll arrow
      // leaves the entry highlighted that was hovered last.
      if (Qt::LeftButton == event->button() && action == this->actionAt(event->position().toPoint())
        && TriggerIfCheckable(action))
      {
        return;
      }

      QMenu::mouseReleaseEvent(event);
    }

  private:
    static bool TriggerIfCheckable(QAction* action)
    {
      if (nullptr == action || !action->isEnabled() || !action->isCheckable())
        return false;

      action->trigger();
      return true;
    }
  };

  std::string GetStatisticKey(const QAbstractItemModel* model, int column)
  {
    return model->headerData(column, Qt::Horizontal, Qt::EditRole).toString().toStdString();
  }
}

/** Hides statistic columns by their keys. Filtering in the model instead of hiding columns
of the view lets the clipboard export follow the table, and lets the alternating column tint
continue across hidden columns. */
class QmitkImageStatisticsWidget::StatisticsFilterProxyModel : public QSortFilterProxyModel
{
public:
  using QSortFilterProxyModel::QSortFilterProxyModel;

  const std::set<std::string>& GetHiddenStatistics() const
  {
    return m_HiddenStatistics;
  }

  void SetHiddenStatistics(const std::set<std::string>& keys)
  {
    this->beginFilterChange();
    m_HiddenStatistics = keys;
    this->endFilterChange(Direction::Columns);
  }

protected:
  bool filterAcceptsColumn(int sourceColumn, const QModelIndex& /*sourceParent*/) const override
  {
    // The first column names the images, masks, labels and time steps.
    return 0 == sourceColumn || !m_HiddenStatistics.contains(GetStatisticKey(this->sourceModel(), sourceColumn));
  }

private:
  std::set<std::string> m_HiddenStatistics;
};

QmitkImageStatisticsWidget::QmitkImageStatisticsWidget(QWidget* parent)
  : QWidget(parent),
    m_Controls(std::make_unique<Ui::QmitkImageStatisticsControls>())
{
  m_Controls->setupUi(this);
  m_imageStatisticsModel = new QmitkImageStatisticsTreeModel(parent);
  m_ProxyModel = new StatisticsFilterProxyModel(this);
  CreateConnections();
  m_Controls->treeViewStatistics->setEnabled(false);
  m_Controls->treeViewStatistics->setModel(m_ProxyModel);
  m_Controls->treeViewStatistics->setItemDelegate(new AlternatingColumnDelegate(this));
  m_ProxyModel->setSourceModel(m_imageStatisticsModel);
  connect(m_imageStatisticsModel, &QmitkImageStatisticsTreeModel::dataAvailable, this, &QmitkImageStatisticsWidget::OnDataAvailable);
  connect(m_imageStatisticsModel, &QmitkImageStatisticsTreeModel::modelChanged, this, [this]()
  {
    m_Controls->treeViewStatistics->expandAll();
    this->ResizeColumnsToContents();
    this->UpdateStatisticsFilterButton();
  });
  connect(m_Controls->checkBoxIgnoreZeroValuedVoxel, &QCheckBox::checkStateChanged,
      this, &QmitkImageStatisticsWidget::IgnoreZeroValuedVoxelStateChanged);
  connect(m_imageStatisticsModel, &QmitkImageStatisticsTreeModel::labelCheckStateChanged,
      this, &QmitkImageStatisticsWidget::LabelCheckStateChanged);
  connect(m_imageStatisticsModel, &QmitkImageStatisticsTreeModel::inputDisplayChanged,
      this, &QmitkImageStatisticsWidget::InputDisplayChanged);
}

QmitkImageStatisticsWidget::~QmitkImageStatisticsWidget()
{
}

void QmitkImageStatisticsWidget::SetDataStorage(mitk::DataStorage* newDataStorage)
{
  m_imageStatisticsModel->SetDataStorage(newDataStorage);
}

void QmitkImageStatisticsWidget::SetImageNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes)
{
  m_imageStatisticsModel->SetImageNodes(nodes);
}

void QmitkImageStatisticsWidget::SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes)
{
  m_imageStatisticsModel->SetMaskNodes(nodes);
}

void QmitkImageStatisticsWidget::Reset()
{
  m_imageStatisticsModel->Clear();
  m_Controls->treeViewStatistics->setEnabled(false);
  m_Controls->buttonCopyImageStatisticsToClipboard->setEnabled(false);
  m_Controls->buttonStatisticsFilter->setEnabled(false);
  m_Controls->checkBoxIgnoreZeroValuedVoxel->setEnabled(false);
}


void QmitkImageStatisticsWidget::SetIgnoreZeroValueVoxel(bool _arg)
{
  m_imageStatisticsModel->SetIgnoreZeroValueVoxel(_arg);
}

bool QmitkImageStatisticsWidget::GetIgnoreZeroValueVoxel() const
{
  return this->m_imageStatisticsModel->GetIgnoreZeroValueVoxel();
}

void QmitkImageStatisticsWidget::SetHistogramNBins(unsigned int nbins)
{
  m_imageStatisticsModel->SetHistogramNBins(nbins);
}

unsigned int QmitkImageStatisticsWidget::GetHistogramNBins() const
{
  return this->m_imageStatisticsModel->GetHistogramNBins();
}

void QmitkImageStatisticsWidget::SetLabelsCheckable(bool checkable)
{
  m_imageStatisticsModel->SetLabelsCheckable(checkable);
}

bool QmitkImageStatisticsWidget::IsLabelChecked(mitk::ImageStatisticsContainer::LabelValueType labelValue) const
{
  return m_imageStatisticsModel->IsLabelChecked(labelValue);
}

void QmitkImageStatisticsWidget::SetHiddenStatistics(const std::set<std::string>& keys)
{
  m_ProxyModel->SetHiddenStatistics(keys);

  // A column that is shown again would otherwise start out with the default width.
  this->ResizeColumnsToContents();
  this->UpdateStatisticsFilterButton();
}

std::set<std::string> QmitkImageStatisticsWidget::GetHiddenStatistics() const
{
  return m_ProxyModel->GetHiddenStatistics();
}

void QmitkImageStatisticsWidget::ChangeHiddenStatistics(const std::set<std::string>& keys)
{
  if (keys == m_ProxyModel->GetHiddenStatistics())
    return;

  this->SetHiddenStatistics(keys);
  emit HiddenStatisticsChanged();
}

void QmitkImageStatisticsWidget::PopulateStatisticsMenu(QMenu* menu, int tableColumn)
{
  menu->clear();
  menu->setToolTipsVisible(true);

  if (tableColumn > 0)
  {
    const auto key = GetStatisticKey(m_ProxyModel, tableColumn);
    const auto name = m_ProxyModel->headerData(tableColumn, Qt::Horizontal, Qt::DisplayRole).toString();

    menu->addAction(QStringLiteral("Show only \"%1\"").arg(name), this, [this, key]()
    {
      // Statistics that are hidden but not present right now stay hidden.
      auto keys = m_ProxyModel->GetHiddenStatistics();

      for (int column = 1; column < m_imageStatisticsModel->columnCount(); ++column)
        keys.insert(GetStatisticKey(m_imageStatisticsModel, column));

      // The model offers only the statistics of the shown data, so the choice has to cover
      // the statistics of other data as well.
      const auto& defaultNames = mitk::ImageStatisticsContainer::ImageStatisticsObject::GetDefaultStatisticNames();
      keys.insert(defaultNames.cbegin(), defaultNames.cend());

      keys.erase(key);
      this->ChangeHiddenStatistics(keys);
    });

    menu->addAction(QStringLiteral("Hide \"%1\"").arg(name), this, [this, key]()
    {
      auto keys = m_ProxyModel->GetHiddenStatistics();
      keys.insert(key);
      this->ChangeHiddenStatistics(keys);
    });
  }

  menu->addAction(QStringLiteral("Show all statistics"), this, [this]()
  {
    this->ChangeHiddenStatistics({});
  });

  menu->addSeparator();

  const auto& hiddenStatistics = m_ProxyModel->GetHiddenStatistics();

  for (int column = 1; column < m_imageStatisticsModel->columnCount(); ++column)
  {
    const auto key = GetStatisticKey(m_imageStatisticsModel, column);

    auto* action = menu->addAction(m_imageStatisticsModel->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString());
    action->setToolTip(m_imageStatisticsModel->headerData(column, Qt::Horizontal, Qt::ToolTipRole).toString());
    action->setCheckable(true);
    action->setChecked(!hiddenStatistics.contains(key));

    connect(action, &QAction::toggled, this, [this, key](bool checked)
    {
      auto keys = m_ProxyModel->GetHiddenStatistics();

      if (checked)
        keys.erase(key);
      else
        keys.insert(key);

      this->ChangeHiddenStatistics(keys);
    });
  }
}

void QmitkImageStatisticsWidget::ResizeColumnsToContents()
{
  auto* treeView = m_Controls->treeViewStatistics;

  // Only expanded rows count, so the names of all labels and time steps fit.
  for (int column = 0; column < treeView->model()->columnCount(); ++column)
    treeView->resizeColumnToContents(column);
}

void QmitkImageStatisticsWidget::UpdateStatisticsFilterButton()
{
  const int statisticCount = m_imageStatisticsModel->columnCount() - 1;
  const int shownCount = m_ProxyModel->columnCount() - 1;

  // Tells that statistics are hidden, as the choice is typically remembered across sessions.
  m_Controls->buttonStatisticsFilter->setText(shownCount < statisticCount
    ? QStringLiteral("Statistics (%1 of %2)").arg(shownCount).arg(statisticCount)
    : QStringLiteral("Statistics"));
}

void QmitkImageStatisticsWidget::CreateConnections()
{
  connect(m_Controls->buttonCopyImageStatisticsToClipboard, &QPushButton::clicked, this, &QmitkImageStatisticsWidget::OnClipboardButtonClicked);

  auto* statisticsMenu = new StayOpenMenu(m_Controls->buttonStatisticsFilter);
  m_Controls->buttonStatisticsFilter->setMenu(statisticsMenu);

  connect(statisticsMenu, &QMenu::aboutToShow, this, [this, statisticsMenu]()
  {
    this->PopulateStatisticsMenu(statisticsMenu);
  });

  auto* header = m_Controls->treeViewStatistics->header();
  header->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(header, &QHeaderView::customContextMenuRequested, this, [this, header](const QPoint& pos)
  {
    StayOpenMenu menu(this);
    this->PopulateStatisticsMenu(&menu, header->logicalIndexAt(pos));
    menu.exec(header->viewport()->mapToGlobal(pos));
  });
}

void QmitkImageStatisticsWidget::OnDataAvailable()
{
  m_Controls->buttonCopyImageStatisticsToClipboard->setEnabled(true);
  m_Controls->buttonStatisticsFilter->setEnabled(true);
  m_Controls->treeViewStatistics->setEnabled(true);
  m_Controls->checkBoxIgnoreZeroValuedVoxel->setEnabled(true);
}

void QmitkImageStatisticsWidget::OnClipboardButtonClicked()
{
  QmitkStatisticsModelToStringConverter converter;
  converter.SetColumnDelimiter('\t');
  converter.SetModel(m_ProxyModel);
  converter.SetRootIndex(m_Controls->treeViewStatistics->rootIndex());
  converter.SetIncludeHeaderData(true);

  QString clipboardAsString = converter.GetString();
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setText(clipboardAsString, QClipboard::Clipboard);
}
