/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPythonSettingsView.h"
#include <ui_QmitkPythonSettingsView.h>

#include <mitkPythonHelper.h>

#include <QmitkRun.h>

#include <array>

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

#include <QtConcurrent>

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

      auto tokens = line.split('=');

      if (tokens.size() == 2 && tokens[0].trimmed() == key)
        return tokens[1].trimmed();
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

  void OpenVenvPath(const QString& venvPath)
  {
    if (QDir(venvPath).exists())
      QDesktopServices::openUrl(QUrl::fromLocalFile(venvPath));
  }
}

class QmitkVenvTreeWidgetItem : public QTreeWidgetItem
{
public:
  QmitkVenvTreeWidgetItem(const QDir& venvDir, const QString& homePath)
  {
    this->setVenvPath(venvDir.path());
    this->setCalculating();
    this->setHomePath(homePath);
  }

  QString name() const
  {
    return this->text(0);
  }

  QString venvPath() const
  {
    return this->data(0, Qt::UserRole).toString();
  }

  QString humanReadableSize() const
  {
    if (this->data(1, Qt::UserRole).isNull())
      return {};

    return this->text(1);
  }

  quint64 totalSize() const
  {
    return this->data(1, Qt::UserRole).value<quint64>();
  }

  QString homePath() const
  {
    return this->text(2);
  }

  void setTotalSize(quint64 totalSize)
  {
    QFont normalFont = this->font(1);
    normalFont.setItalic(false);

    this->setData(1, Qt::UserRole, QVariant::fromValue(totalSize));
    this->setText(1, GetHumanReadableSize(totalSize));
    this->setFont(1, normalFont);
  }

  bool operator<(const QTreeWidgetItem& other) const override
  {
    auto otherItem = dynamic_cast<const QmitkVenvTreeWidgetItem*>(&other);

    if (otherItem != nullptr && this->treeWidget() != nullptr)
    {
      if (this->treeWidget()->sortColumn() == 1)
        return this->totalSize() < otherItem->totalSize();
    }

    return QTreeWidgetItem::operator<(other);
  }

private:
  void setVenvPath(const QString& venvPath)
  {
    const auto nativeVenvPath = QDir::toNativeSeparators(venvPath);

    this->setData(0, Qt::UserRole, nativeVenvPath);
    this->setText(0, QDir(venvPath).dirName());
    this->setToolTip(0, nativeVenvPath);
  }

  void setCalculating()
  {
    QFont italicFont;
    italicFont.setItalic(true);

    this->setText(1, "Calculating...");
    this->setFont(1, italicFont);
  }

  void setHomePath(const QString& homePath)
  {
    this->setText(2, QDir::toNativeSeparators(homePath));
  }
};

namespace
{
  QList<QmitkVenvTreeWidgetItem*> GetSelectedVenvItems(const QTreeWidget* treeWidget)
  {
    const auto items = treeWidget->selectedItems();
    QList<QmitkVenvTreeWidgetItem*> venvItems;

    for (auto item : items)
    {
      if (auto venvItem = dynamic_cast<QmitkVenvTreeWidgetItem*>(item); venvItem != nullptr)
        venvItems.push_back(venvItem);
    }

    return venvItems;
  }
}

const std::string QmitkPythonSettingsView::VIEW_ID = "org.mitk.views.pythonsettings";

QmitkPythonSettingsView::QmitkPythonSettingsView(QObject*)
  : m_Ui(new Ui::QmitkPythonSettingsView)
{
}

QmitkPythonSettingsView::~QmitkPythonSettingsView()
{
}

void QmitkPythonSettingsView::CreateQtPartControl(QWidget* parent)
{
  using Self = QmitkPythonSettingsView;

  m_Ui->setupUi(parent);

  connect(m_Ui->venvsTreeWidget, &QTreeWidget::itemDoubleClicked, [](QTreeWidgetItem* item, int) {
    if (auto venvItem = dynamic_cast<const QmitkVenvTreeWidgetItem*>(item); venvItem != nullptr)
      OpenVenvPath(venvItem->venvPath());
  });

  connect(m_Ui->venvsTreeWidget, &QTreeWidget::itemSelectionChanged, [this]() {
    m_Ui->deleteVenvsButton->setEnabled(!m_Ui->venvsTreeWidget->selectedItems().empty());
  });

  connect(m_Ui->deleteVenvsButton, &QPushButton::clicked, this, &Self::DeleteSelectedVenvs);
  connect(m_Ui->refreshVenvsButton, &QPushButton::clicked, this, &Self::RefreshVenvsTreeWidget);

  this->RefreshVenvsTreeWidget();
}

void QmitkPythonSettingsView::DeleteSelectedVenvs()
{
  const auto selectedItems = GetSelectedVenvItems(m_Ui->venvsTreeWidget);

  if (selectedItems.empty())
    return;

  const auto answer = QMessageBox::question(
    nullptr,
    "Delete selected virtual environments",
    QString(
      "<h3 %1>Delete selected virtual environments?</h3>"
      "<p %1><em>Warning:</em> This action cannot be undone. Deleting active "
      "environments may cause the application to crash.</p>").arg(LINE_HEIGHT_STYLE),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (answer != QMessageBox::Yes)
    return;

  for (const auto* item : selectedItems)
  {
    const auto venvPath = item->venvPath();

    QmitkRunAsyncBlocking("Delete selected virtual environments", QString("Deleting \"%1\"...").arg(venvPath), [&venvPath]() {
      QDir(venvPath).removeRecursively();
    });
  }

  this->RefreshVenvsTreeWidget();
}

void QmitkPythonSettingsView::RefreshVenvsTreeWidget()
{
  this->PopulateVenvsTreeWidget();
  this->CalculateAllVenvSizes();
}

void QmitkPythonSettingsView::PopulateVenvsTreeWidget()
{
  m_Ui->venvsTreeWidget->clearSelection();
  m_Ui->venvsTreeWidget->clear();

  const QDir rootDir(mitk::PythonHelper::GetVirtualEnvBasePath().parent_path());
  QDirIterator baseDirIt(rootDir.path(), QDir::Dirs);

  while (baseDirIt.hasNext())
  {
    QDirIterator venvDirIt(baseDirIt.next(), QDir::Dirs);

    while (venvDirIt.hasNext())
    {
      const auto venvDir = QDir(venvDirIt.next());
      const auto venvConfigPath = venvDir.filePath("pyvenv.cfg");

      if (QFile::exists(venvConfigPath))
      {
        if (const auto homePath = ReadConfigValue(venvConfigPath, "home"); !homePath.isEmpty())
        {
          auto item = new QmitkVenvTreeWidgetItem(venvDir, homePath);
          m_Ui->venvsTreeWidget->addTopLevelItem(item);
        }
      }
    }
  }

  m_Ui->venvsTreeWidget->sortByColumn(2, Qt::AscendingOrder);
}

void QmitkPythonSettingsView::CalculateAllVenvSizes()
{
  const int venvCount = m_Ui->venvsTreeWidget->topLevelItemCount();

  for (int itemIndex = 0; itemIndex < venvCount; ++itemIndex)
  {
    auto item = dynamic_cast<QmitkVenvTreeWidgetItem*>(m_Ui->venvsTreeWidget->topLevelItem(itemIndex));

    if (item == nullptr)
      continue;

    auto future = QtConcurrent::run([item]() {
      const auto totalSize = CalculateTotalDirectorySize(item->venvPath());

      QMetaObject::invokeMethod(item->treeWidget(), [item, totalSize]() {
        item->setTotalSize(totalSize);
      });
    });
  }
}

void QmitkPythonSettingsView::SetFocus()
{
  m_Ui->venvsTreeWidget->setFocus();
}
