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

#include "QmitkNativeFileOpenDialog.h"

#include "QmitkIOUtil.h"

#include "mitkCoreServices.h"
#include "mitkIMimeTypeProvider.h"

#include <QSet>

class QmitkNativeFileOpenDialogPrivate
{

public:

  QWidget* parent;
  QString caption;
  QString dir;
  QFileDialog::Options options;
  QString filter;
  QStringList selectedFiles;

  QString GetFilter()
  {
    QString filters;

    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
    std::vector<std::string> categories = mimeTypeProvider->GetCategories();
    std::sort(categories.begin(), categories.end());
    for (std::vector<std::string>::iterator cat = categories.begin(); cat != categories.end(); ++cat)
    {
      QSet<QString> filterExtensions;
      std::vector<std::string> mimeTypes = mimeTypeProvider->GetMimeTypesForCategory(*cat);
      for (std::vector<std::string>::iterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
      {
        std::vector<std::string> extensions = mimeTypeProvider->GetExtensions(*mt);
        for (std::vector<std::string>::iterator ext = extensions.begin(); ext != extensions.end(); ++ext)
        {
          filterExtensions << QString::fromStdString(*ext);
        }
      }

      QString filter = QString::fromStdString(*cat) + " (";
      foreach(const QString& extension, filterExtensions)
      {
        filter += "*." + extension + " ";
      }
      filter = filter.replace(filter.size()-1, 1, ')');
      filters += ";;" + filter;
    }
    filters.prepend("All (*.*)");
    return filters;
  }
};

QmitkNativeFileOpenDialog::QmitkNativeFileOpenDialog(QWidget* parent)
  : d(new QmitkNativeFileOpenDialogPrivate)
{
  d->parent = parent;
  d->filter = d->GetFilter();
}

QmitkNativeFileOpenDialog::~QmitkNativeFileOpenDialog()
{
}

void QmitkNativeFileOpenDialog::SetCaption(const QString& caption)
{
  d->caption = caption;
}

void QmitkNativeFileOpenDialog::SetDir(const QString& dir)
{
  d->dir = dir;
}

void QmitkNativeFileOpenDialog::SetOptions(QFileDialog::Options options)
{
  d->options = options;
}

void QmitkNativeFileOpenDialog::Exec()
{
  d->selectedFiles = QFileDialog::getOpenFileNames(d->parent, d->caption, d->dir, d->filter, NULL, d->options);
}

QStringList QmitkNativeFileOpenDialog::GetSelectedFiles() const
{
  return d->selectedFiles;
}

QList<mitk::BaseData::Pointer> QmitkNativeFileOpenDialog::ReadBaseData(mitk::DataStorage* ds) const
{
  return QmitkIOUtil::LoadFiles(d->selectedFiles, ds);
}
