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
#include <mitkBaseData.h>
#include <mitkCommon.h>
#include <mitkDataNode.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>

// Qt
#include <QList>
#include <QPair>
#include <QScopedPointer>
#include <QString>

class QWidget;
class QString;
class QStringList;

namespace mitk
{
  class DataStorage;
  class MimeType;
  struct IFileReader;
}

/**
 * @brief QmitkIOUtil Provides static helper methods to open and save files with Qt dialogs.
 */
class MITKQTWIDGETS_EXPORT QmitkIOUtil : public mitk::IOUtil
{
public:
  class MITKQTWIDGETS_EXPORT SaveFilter
  {
  public:
    static mitk::MimeType ALL_MIMETYPE();

    SaveFilter(const SaveFilter &other);

    SaveFilter(const SaveInfo &saveInfo);

    SaveFilter &operator=(const SaveFilter &other);

    std::vector<mitk::MimeType> GetMimeTypes() const;
    QString GetFilterForMimeType(const std::string &mimeType) const;
    mitk::MimeType GetMimeTypeForFilter(const QString &filter) const;
    QString GetDefaultFilter() const;
    QString GetDefaultExtension() const;
    mitk::MimeType GetDefaultMimeType() const;
    QString ToString() const;
    int Size() const;
    bool IsEmpty() const;

    bool ContainsMimeType(const std::string &mimeType);

  private:
    struct Impl;
    QScopedPointer<Impl> d;
  };

  /**
   * @brief GetFilterString
   * @return
   */
  static QString GetFileOpenFilterString();

  /**
   * @brief Loads the specified files
   *
   * This methods tries to load all specified files and pop-ups dialog boxes if further
   * user input is required (e.g. ambiguous mime-types or reader options).
   *
   * If the provided DataStorage is not nullptr, some files will be added to it automatically,
   * dependeing on the IFileReader used.
   *
   * @param files A list of files to load.
   * @param ds An optional data storage passed to IFileReader instances
   * @return A list of BaseData instances which have not already been added to the data storage.
   */
  static QList<mitk::BaseData::Pointer> Load(const QStringList &paths, QWidget *parent = nullptr);

  static mitk::DataStorage::SetOfObjects::Pointer Load(const QStringList &paths,
                                                       mitk::DataStorage &storage,
                                                       QWidget *parent = nullptr);

  static QList<mitk::BaseData::Pointer> Load(const QString &path, QWidget *parent = nullptr);

  static mitk::DataStorage::SetOfObjects::Pointer Load(const QString &path,
                                                       mitk::DataStorage &storage,
                                                       QWidget *parent = nullptr);

  using mitk::IOUtil::Load;

  static QString Save(const mitk::BaseData *data,
                      const QString &defaultBaseName,
                      const QString &defaultPath = QString(),
                      QWidget *parent = nullptr,
                      bool setPathProperty = false);

  /**
   * @brief Save a list of BaseData objects using a "File Save Dialog".
   *
   * For each element in the \c data vector, the following algorithm is
   * used to find a IFileWriter instance for writing the BaseData object.
   *
   * First, the user is prompted to select file names for each BaseData object. This
   * is equivalent to choosing a specific mime-type, either by selecting a filter
   * in the save dialog or by explicitly providing a file name extension:
   * <ol>
   * <li>Get a list of registered IFileWriter objects for the current BaseData object.
   *     If no writers are found, a message box displays a warning and
   *     the process starts from the beginning for the next BaseData object.</li>
   * <li>A QFileDialog for prompting the user to select a file name is opened.
   *     The mime-type associated with each IFileWriter object is used to create
   *     a filter for file name extensions.
   *     The best IFileWriter (see FileWriterSelector) for the current BaseData object
   *     defines the default file name suffix via its associated mime-type. If the
   *     file name is empty (the user cancelled the dialog), the remaining
   *     BaseData objects are skipped.
   * <li>The file name is matched against valid mime-types. The first mime-type
   *     which accepts the file name is associated with the current BaseData object.
   *     If no mime-type accepts the supplied file name and the file already
   *     exists, the process starts from the beginning with the next BaseData object.
   *     Otherwise, if the selected filter is the special "all" filter and the
   *     file name does not contain periods (which may or may not mark the start of
   *     a file extension), the current BaseData object is associated with the
   *     default mime-type. If the selected filter is not the special "all" filter,
   *     the mime-type for this filter is associated with the current BaseData object.
   *     The default extension of the associated mime-type is then appended to the
   *     supplied file name.
   * <li>The selected/derived file name and associated mime-type is stored in a list
   *     and the process starts from the beginning for the next BaseData object.</li>
   * </ol>
   *
   * In the second phase, each BaseData object is saved to disk using the specified
   * file name and mime-type, according to the following procedure:
   * <ol>
   * <li>If multiple IFileWriter objects are compatible with the current base data
   *     object or if the single compatible IFileWriter provides configuration
   *     options, a dialog window containing a list of IFileWriter objects and
   *     configurable options is displayed. If the dialog is cancelled by the user,
   *     neither the current nor the remaining base data objects are saved to disk.
   *     If the user previously in this phase enabled the "remember options" checkbox
   *     of the dialog, then the dialog is not shown for base data objects with the
   *     same data type and associated mime-type if the file writer instance reports
   *     a higher or equal confidence level for the current base data object.</li>
   * <li>The selected writer (either the only available one or the user selected one)
   *     is used to write the base data object to disk. On failure, an error is
   *     reported and the second phase continues with the next base data object.</li>
   * </ol>
   *
   * @param data
   * @param defaultBaseNames
   * @param defaultPath
   * @param parent
   * @return
   */
  static QStringList Save(const std::vector<const mitk::BaseData *> &data,
                          const QStringList &defaultBaseNames,
                          const QString &defaultPath = QString(),
                          QWidget *parent = nullptr,
                          bool setPathProperty = false);

  using mitk::IOUtil::Save;

  /**
   * @brief SaveBaseDataWithDialog Convenience method to save any data with a Qt dialog.
   * @param data BaseData holding the data you wish to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the
   * screen.
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static void SaveBaseDataWithDialog(mitk::BaseData *data, std::string fileName, QWidget *parent = nullptr));

  /**
   * @brief SaveSurfaceWithDialog Convenience method to save a surface with a Qt dialog.
   * @param surface The surface to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the
   * screen.
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static void SaveSurfaceWithDialog(mitk::Surface::Pointer surface,
                                               std::string fileName = "",
                                               QWidget *parent = nullptr));

  /**
   * @brief SaveImageWithDialog Convenience method to save an image with a Qt dialog.
   * @param image The image to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the
   * screen.
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static void SaveImageWithDialog(mitk::Image::Pointer image,
                                             std::string fileName = "",
                                             QWidget *parent = nullptr));

  /**
   * @brief SavePointSetWithDialog Convenience method to save a pointset with a Qt dialog.
   * @param pointset The pointset to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the
   * screen.
   * @deprecatedSince{2014_10} Use Save() instead.
   */
  DEPRECATED(static void SavePointSetWithDialog(mitk::PointSet::Pointer pointset,
                                                std::string fileName = "",
                                                QWidget *parent = nullptr));

private:
  struct Impl;
};

#endif // _QmitkIOUtil__h_
