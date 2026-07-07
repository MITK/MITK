/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPythonEnvironmentsView.h"
#include <ui_QmitkPythonEnvironmentsView.h>

#include <mitkPythonHelper.h>

#include <QmitkRun.h>
#include <QmitkStyleManager.h>

#include <array>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFont>
#include <QList>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTreeWidgetItem>
#include <QUrl>

#include <QThreadPool>

namespace
{
  constexpr auto LINE_HEIGHT_STYLE = "style='line-height: 1.25'";

  QString ReadConfigValue(const QString& fileName, const QString& key)
  {
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      MITK_WARN << "Cannot open file: " << fileName.toStdString();
      return {};
    }

    QTextStream stream(&file);

    while (!stream.atEnd())
    {
      auto line = stream.readLine();

      if (line.isEmpty() || line.startsWith('#'))
        continue;

      const auto separatorIndex = line.indexOf('=');

      if (separatorIndex < 0)
        continue;

      if (line.left(separatorIndex).trimmed() == key)
        return line.mid(separatorIndex + 1).trimmed();
    }

    MITK_WARN << "Key \"" << key.toStdString() << "\" not found in file: " << fileName.toStdString();
    return {};
  }

  quint64 CalculateTotalDirectorySize(const QString& dirPath)
  {
    quint64 totalSize = 0;
    QDirIterator fileIt(dirPath, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while (fileIt.hasNext())
    {
      fileIt.next();
      totalSize += static_cast<quint64>(fileIt.fileInfo().size());
    }

    return totalSize;
  }

  QString GetHumanReadableSize(quint64 bytes)
  {
    static std::array<const char*, 4> sizes = { "B", "KB", "MB", "GB" };
    constexpr int maxSizeIndex = sizes.size() - 1;

    int sizeIndex = 0;
    double num = bytes;

    while (num >= 1024.0 && sizeIndex < maxSizeIndex)
    {
      num /= 1024.0;
      ++sizeIndex;
    }

    return QString::number(num, 'f', 2) + " " + sizes[sizeIndex];
  }

  void OpenVirtualEnvPath(const QString& virtualEnvPath)
  {
    if (QDir(virtualEnvPath).exists())
      QDesktopServices::openUrl(QUrl::fromLocalFile(virtualEnvPath));
  }
}

class QmitkVirtualEnvTreeWidgetItem : public QTreeWidgetItem
{
public:
  QmitkVirtualEnvTreeWidgetItem(const QDir& virtualEnvDir, const QString& homePath, bool isOwn)
    : m_IsOwn(isOwn),
      m_Name(virtualEnvDir.dirName())
  {
    if (m_IsOwn)
    {
      this->setText(0, QStringLiteral("✓"));
      this->setTextAlignment(0, Qt::AlignCenter);
    }

    this->setVirtualEnvPath(virtualEnvDir.path());
    this->setText(1, m_Name);
    this->setCalculating();
    this->setHomePath(homePath);
  }

  QString name() const
  {
    return m_Name;
  }

  QString virtualEnvPath() const
  {
    return this->data(1, Qt::UserRole).toString();
  }

  QString humanReadableSize() const
  {
    if (this->data(2, Qt::UserRole).isNull())
      return {};

    return this->text(2);
  }

  quint64 totalSize() const
  {
    return this->data(2, Qt::UserRole).value<quint64>();
  }

  QString homePath() const
  {
    return this->text(3);
  }

  bool isOwn() const
  {
    return m_IsOwn;
  }

  void setTotalSize(quint64 totalSize)
  {
    QFont font = this->font(2);
    font.setItalic(false);

    this->setData(2, Qt::UserRole, QVariant::fromValue(totalSize));
    this->setText(2, GetHumanReadableSize(totalSize));
    this->setFont(2, font);
  }

  bool operator<(const QTreeWidgetItem& other) const override
  {
    auto otherItem = dynamic_cast<const QmitkVirtualEnvTreeWidgetItem*>(&other);

    if (otherItem != nullptr && this->treeWidget() != nullptr)
    {
      if (this->treeWidget()->sortColumn() == 2)
        return this->totalSize() < otherItem->totalSize();
    }

    return QTreeWidgetItem::operator<(other);
  }

private:
  void setVirtualEnvPath(const QString& virtualEnvPath)
  {
    const auto nativeVirtualEnvPath = QDir::toNativeSeparators(virtualEnvPath);

    this->setData(1, Qt::UserRole, nativeVirtualEnvPath);
    this->setToolTip(1, nativeVirtualEnvPath);
  }

  void setCalculating()
  {
    QFont font = this->font(2);
    font.setItalic(true);

    this->setText(2, "Calculating...");
    this->setFont(2, font);
  }

  void setHomePath(const QString& homePath)
  {
    this->setText(3, QDir::toNativeSeparators(homePath));
  }

  bool m_IsOwn;
  QString m_Name;
};

namespace
{
  QList<QmitkVirtualEnvTreeWidgetItem*> GetSelectedVirtualEnvItems(const QTreeWidget* treeWidget)
  {
    const auto items = treeWidget->selectedItems();
    QList<QmitkVirtualEnvTreeWidgetItem*> virtualEnvItems;

    for (auto item : items)
    {
      if (auto virtualEnvItem = dynamic_cast<QmitkVirtualEnvTreeWidgetItem*>(item); virtualEnvItem != nullptr)
        virtualEnvItems.push_back(virtualEnvItem);
    }

    return virtualEnvItems;
  }

  QmitkVirtualEnvTreeWidgetItem* FindVirtualEnvItem(const QTreeWidget* treeWidget, const QString& virtualEnvPath)
  {
    const int virtualEnvCount = treeWidget->topLevelItemCount();

    for (int itemIndex = 0; itemIndex < virtualEnvCount; ++itemIndex)
    {
      auto item = dynamic_cast<QmitkVirtualEnvTreeWidgetItem*>(treeWidget->topLevelItem(itemIndex));

      if (item != nullptr && item->virtualEnvPath() == virtualEnvPath)
        return item;
    }

    return nullptr;
  }
}

const std::string QmitkPythonEnvironmentsView::VIEW_ID = "org.mitk.views.pythonenvironments";

QmitkPythonEnvironmentsView::QmitkPythonEnvironmentsView(QObject*)
  : m_Ui(std::make_unique<Ui::QmitkPythonEnvironmentsView>())
{
}

QmitkPythonEnvironmentsView::~QmitkPythonEnvironmentsView()
{
  // Make in-flight size workers finish before m_Ui is destroyed and before
  // ~QObject removes our pending queued events. clear() drops not-yet-started
  // runnables; waitForDone() blocks on those already running.
  m_SizeThreadPool.clear();
  m_SizeThreadPool.waitForDone();
}

void QmitkPythonEnvironmentsView::CreateQtPartControl(QWidget* parent)
{
  using Self = QmitkPythonEnvironmentsView;

  m_Ui->setupUi(parent);

  m_Ui->deleteVirtualEnvsButton->setIcon(QmitkStyleManager::ThemeIcon(QLatin1String(":/QmitkPythonEnvironmentsView/trash.svg")));
  m_Ui->refreshVirtualEnvsButton->setIcon(QmitkStyleManager::ThemeIcon(QLatin1String(":/QmitkPythonEnvironmentsView/arrow-rotate-right.svg")));

  m_Ui->virtualEnvsTreeWidget->setColumnWidth(0, 40);
  m_Ui->virtualEnvsTreeWidget->setColumnWidth(1, 250);

  connect(m_Ui->virtualEnvsTreeWidget, &QTreeWidget::itemDoubleClicked, [](QTreeWidgetItem* item, int) {
    if (auto virtualEnvItem = dynamic_cast<const QmitkVirtualEnvTreeWidgetItem*>(item); virtualEnvItem != nullptr)
      OpenVirtualEnvPath(virtualEnvItem->virtualEnvPath());
  });

  connect(m_Ui->virtualEnvsTreeWidget, &QTreeWidget::itemSelectionChanged, [this]() {
    m_Ui->deleteVirtualEnvsButton->setEnabled(!m_Ui->virtualEnvsTreeWidget->selectedItems().empty());
  });

  connect(m_Ui->deleteVirtualEnvsButton, &QPushButton::clicked, this, &Self::DeleteSelectedVirtualEnvs);
  connect(m_Ui->refreshVirtualEnvsButton, &QPushButton::clicked, this, &Self::RefreshVirtualEnvsTreeWidget);

  this->RefreshVirtualEnvsTreeWidget();
}

void QmitkPythonEnvironmentsView::DeleteSelectedVirtualEnvs()
{
  const auto selectedItems = GetSelectedVirtualEnvItems(m_Ui->virtualEnvsTreeWidget);

  if (selectedItems.empty())
    return;

  // IsAnyVirtualEnvModuleLoaded only sees modules loaded into THIS process,
  // so we can only gate venvs that belong to the running MITK instance. Venvs
  // owned by other instances are covered by the warning text below instead.
  QStringList loadedNames;

  for (const auto* item : selectedItems)
  {
    if (!item->isOwn())
      continue;

    const fs::path path(item->virtualEnvPath().toStdString());

    if (mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded(path))
      loadedNames.append(item->name());
  }

  if (!loadedNames.isEmpty())
  {
    const auto appName = QCoreApplication::applicationName();
    const auto restartTarget = appName.isEmpty()
      ? QStringLiteral("this application")
      : appName;

    QMessageBox::information(
      nullptr,
      "Delete selected virtual environments",
      QString(
        "<h3 %1>Cannot delete in-use virtual environments</h3>"
        "<p %1>The following virtual environments cannot be deleted because "
        "Python modules from them are still loaded:</p>"
        "<ul %1><li>%2</li></ul>"
        "<p %1>Restart %3 and try again.</p>")
        .arg(LINE_HEIGHT_STYLE, loadedNames.join("</li><li>"), restartTarget));
    return;
  }

  const auto answer = QMessageBox::question(
    nullptr,
    "Delete selected virtual environments",
    QString(
      "<h3 %1>Delete selected virtual environments?</h3>"
      "<p %1><em>Warning:</em> This action cannot be undone. If a selected "
      "virtual environment is in use by another running MITK instance, "
      "deleting it may cause that instance to crash.</p>").arg(LINE_HEIGHT_STYLE),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (answer != QMessageBox::Yes)
    return;

  // Snapshot paths on the GUI thread; QTreeWidgetItem is not safe to touch
  // from the worker thread.
  QStringList virtualEnvPaths;
  virtualEnvPaths.reserve(selectedItems.size());

  for (const auto* item : selectedItems)
    virtualEnvPaths.append(item->virtualEnvPath());

  QStringList failedPaths;

  QmitkRunAsyncBlocking(
    "Delete selected virtual environments",
    "Deleting virtual environments...",
    [&virtualEnvPaths, &failedPaths]() {
      for (const auto& pathString : virtualEnvPaths)
      {
        const fs::path path(pathString.toStdString());

        if (!mitk::PythonHelper::RemoveVirtualEnv(path))
          failedPaths.append(pathString);
      }
    });

  if (!failedPaths.isEmpty())
  {
    QMessageBox::warning(
      nullptr,
      "Delete selected virtual environments",
      QString(
        "<h3 %1>Some virtual environments could not be deleted</h3>"
        "<p %1>The following could not be removed. They may be in use by "
        "another running MITK instance, or may not be writable:</p>"
        "<ul %1><li>%2</li></ul>")
        .arg(LINE_HEIGHT_STYLE, failedPaths.join("</li><li>")));
  }

  this->RefreshVirtualEnvsTreeWidget();
}

void QmitkPythonEnvironmentsView::RefreshVirtualEnvsTreeWidget()
{
  this->PopulateVirtualEnvsTreeWidget();
  this->CalculateAllVirtualEnvSizes();
}

void QmitkPythonEnvironmentsView::PopulateVirtualEnvsTreeWidget()
{
  m_Ui->virtualEnvsTreeWidget->clearSelection();
  m_Ui->virtualEnvsTreeWidget->clear();

  const auto ownBasePath = mitk::PythonHelper::GetVirtualEnvBasePath();

  if (ownBasePath.empty())
    return;

  const QDir rootDir(QString::fromStdString(ownBasePath.parent_path().string()));

  if (!rootDir.exists())
    return;

  const auto ownInstanceDir = QDir(QString::fromStdString(ownBasePath.string())).canonicalPath();

  QDirIterator instanceDirIt(rootDir.path(), QDir::Dirs | QDir::NoDotAndDotDot);

  while (instanceDirIt.hasNext())
  {
    const auto instanceDirPath = instanceDirIt.next();
    const auto canonicalInstanceDir = QDir(instanceDirPath).canonicalPath();
    const bool isOwn = !canonicalInstanceDir.isEmpty() && canonicalInstanceDir == ownInstanceDir;

    QDirIterator virtualEnvDirIt(instanceDirPath, QDir::Dirs | QDir::NoDotAndDotDot);

    while (virtualEnvDirIt.hasNext())
    {
      const auto virtualEnvDir = QDir(virtualEnvDirIt.next());
      const auto virtualEnvConfigPath = virtualEnvDir.filePath("pyvenv.cfg");

      if (QFile::exists(virtualEnvConfigPath))
      {
        if (const auto homePath = ReadConfigValue(virtualEnvConfigPath, "home"); !homePath.isEmpty())
        {
          auto item = new QmitkVirtualEnvTreeWidgetItem(virtualEnvDir, homePath, isOwn);
          m_Ui->virtualEnvsTreeWidget->addTopLevelItem(item);
        }
      }
    }
  }

  m_Ui->virtualEnvsTreeWidget->sortByColumn(3, Qt::AscendingOrder);
}

void QmitkPythonEnvironmentsView::CalculateAllVirtualEnvSizes()
{
  // Each refresh starts a new generation. Workers from older generations
  // still post their results, but SetVirtualEnvSize ignores them so they
  // cannot touch tree items that were rebuilt in the meantime.
  const int currentGeneration = ++m_RefreshGeneration;
  const int virtualEnvCount = m_Ui->virtualEnvsTreeWidget->topLevelItemCount();

  for (int itemIndex = 0; itemIndex < virtualEnvCount; ++itemIndex)
  {
    auto item = dynamic_cast<QmitkVirtualEnvTreeWidgetItem*>(m_Ui->virtualEnvsTreeWidget->topLevelItem(itemIndex));

    if (item == nullptr)
      continue;

    const auto virtualEnvPath = item->virtualEnvPath();

    m_SizeThreadPool.start([this, virtualEnvPath, currentGeneration]() {
      const auto totalSize = CalculateTotalDirectorySize(virtualEnvPath);

      // Posting back via invokeMethod with `this` as receiver is safe: if
      // the view is destroyed before this fires, Qt drops the queued event.
      QMetaObject::invokeMethod(this, [this, virtualEnvPath, totalSize, currentGeneration]() {
        this->SetVirtualEnvSize(virtualEnvPath, totalSize, currentGeneration);
      });
    });
  }
}

void QmitkPythonEnvironmentsView::SetVirtualEnvSize(const QString& virtualEnvPath, quint64 totalSize, int generation)
{
  if (generation != m_RefreshGeneration)
    return;

  if (auto* item = FindVirtualEnvItem(m_Ui->virtualEnvsTreeWidget, virtualEnvPath))
    item->setTotalSize(totalSize);
}

void QmitkPythonEnvironmentsView::SetFocus()
{
  m_Ui->virtualEnvsTreeWidget->setFocus();
}
