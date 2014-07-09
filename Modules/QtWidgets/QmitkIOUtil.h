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


#ifndef _QmitkIOUtil__h_
#define _QmitkIOUtil__h_

#include "MitkQtWidgetsExports.h"

// std
#include <string>

// mitk includes
#include <mitkCommon.h>
#include <mitkBaseData.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkIOUtil.h>
#include <mitkFileWriterWithInformation.h>

//Qt
#include <QList>
#include <QPair>

class QWidget;
class QString;
class QStringList;

namespace mitk {
class DataStorage;
struct IFileReader;
}

/**
 * @brief QmitkIOUtil Provides static helper methods to open and save files with Qt dialogs.
 */
class QMITK_EXPORT QmitkIOUtil : public mitk::IOUtil
{

public:

  class QMITK_EXPORT SaveFilter
  {
  public:

    static std::string ALL_MIMETYPE;

    SaveFilter(const SaveFilter& other);

    SaveFilter(const std::string& baseDataType);
    SaveFilter(const mitk::BaseData* baseData);

    SaveFilter& operator=(const SaveFilter& other);

    QString GetFilterForMimeType(const std::string& mimeType) const;
    std::string GetMimeTypeForFilter(const QString& filter) const;
    QString GetDefaultFilter() const;
    std::string GetDefaultMimeType() const;
    QString ToString() const;
    int Size() const;
    bool IsEmpty() const;

    bool ContainsMimeType(const std::string& mimeType);

  private:

    struct Impl;
    QScopedPointer<Impl> d;
  };

  /**
   * @brief GetFilterString
   * @return
   */
  static QString GetFileOpenFilterString();

  static SaveFilter GetFileSaveFilter(const mitk::BaseData* baseData);
  static SaveFilter GetFileSaveFilter(const std::string& baseDataType);

  /**
   * @brief Loads the specified files
   *
   * This methods tries to load all specified files and pop-ups dialog boxes if further
   * user input is required (e.g. ambiguous mime-types or reader options).
   *
   * If the provided DataStorage is not NULL, some files will be added to it automatically,
   * dependeing on the IFileReader used.
   *
   * @param files A list of files to load.
   * @param ds An optional data storage passed to IFileReader instances
   * @return A list of BaseData instances which have not already been added to the data storage.
   */
  static QList<mitk::BaseData::Pointer> Load(const QStringList& paths, QWidget* parent = NULL);

  static mitk::DataStorage::SetOfObjects::Pointer Load(const QStringList& paths, mitk::DataStorage& storage,
                                                       QWidget* parent = NULL);

  static QList<mitk::BaseData::Pointer> Load(const QString& path, QWidget* parent = NULL);

  static mitk::DataStorage::SetOfObjects::Pointer Load(const QString& path, mitk::DataStorage& storage,
                                                       QWidget* parent = NULL);

  using mitk::IOUtil::Load;

  static QString Save(const mitk::BaseData* data, const QString& defaultBaseName,
                      const QString& defaultPath = QString(), QWidget* parent = NULL);

  static QStringList Save(const std::vector<const mitk::BaseData*>& data, const QStringList& defaultBaseNames,
                          const QString& defaultPath = QString(), QWidget* parent = NULL);

  using mitk::IOUtil::Save;

  /**
   * @brief SaveBaseDataWithDialog Convenience method to save any data with a Qt dialog.
   * @param data BaseData holding the data you wish to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   * @deprecatedSince{2014_03} Use Save() instead.
   */
  DEPRECATED(static void SaveBaseDataWithDialog(mitk::BaseData *data, std::string fileName, QWidget* parent = NULL));

  /**
   * @brief SaveSurfaceWithDialog Convenience method to save a surface with a Qt dialog.
   * @param surface The surface to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   * @deprecatedSince{2014_03} Use Save() instead.
   */
  DEPRECATED(static void SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName = "", QWidget* parent = NULL));

  /**
   * @brief SaveImageWithDialog Convenience method to save an image with a Qt dialog.
   * @param image The image to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   * @deprecatedSince{2014_03} Use Save() instead.
   */
  DEPRECATED(static void SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName = "", QWidget* parent = NULL));

  /**
   * @brief SavePointSetWithDialog Convenience method to save a pointset with a Qt dialog.
   * @param pointset The pointset to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   * @deprecatedSince{2014_03} Use Save() instead.
   */
  DEPRECATED(static void SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName = "", QWidget* parent = NULL));

private:

  struct Impl;

};

#endif // _QmitkIOUtil__h_
