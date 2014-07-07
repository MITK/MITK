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

  /**
   * @brief GetFilterString
   * @return
   */
  static QString GetFilterString();

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
  static QList<mitk::BaseData::Pointer> Load(const QStringList& paths);

  static mitk::DataStorage::SetOfObjects::Pointer Load(const QStringList& paths, mitk::DataStorage& storage);

  static QList<mitk::BaseData::Pointer> Load(const QString& path);

  static mitk::DataStorage::SetOfObjects::Pointer Load(const QString& path, mitk::DataStorage& storage);

  using mitk::IOUtil::Load;

  /**
   * @brief SaveBaseDataWithDialog Convenience method to save any data with a Qt dialog.
   * @param data BaseData holding the data you wish to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   */
  static void SaveBaseDataWithDialog(mitk::BaseData *data, std::string fileName, QWidget* parent = NULL);

  /**
   * @brief SaveSurfaceWithDialog Convenience method to save a surface with a Qt dialog.
   * @param surface The surface to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   */
  static void SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName = "", QWidget* parent = NULL);

  /**
   * @brief SaveImageWithDialog Convenience method to save an image with a Qt dialog.
   * @param image The image to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   */
  static void SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName = "", QWidget* parent = NULL);

  /**
   * @brief SavePointSetWithDialog Convenience method to save a pointset with a Qt dialog.
   * @param pointset The pointset to save.
   * @param fileName The file name where to save the data (including path and extension).
   * @param parent An optional QWidget as parent. If no parent is supplied, the QFileDialog can occur anywhere on the screen.
   */
  static void SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName = "", QWidget* parent = NULL);

protected:

  /**
   * @brief GetFileNameWithQDialog Opens a QDialog and returns the filename.
   * @param caption Caption for the QDialog.
   * @param defaultFilename Default filename (e.g. "NewImage.nrrd" for images).
   * @param filter Filters the data according to data types (e.g. *.nrrd; *.png; etc. for images).
   * @param selectedFilter Default selected filter for the data.
   * @return The file name as QString.
   */
  static QString GetFileNameWithQDialog(QString caption, QString defaultFilename, QString filter, QString* selectedFilter = 0, QWidget* parent = NULL);

  /**
   * @brief SaveToFileWriter Internal helper method to save data with writer's which have been supplied by extensions (e.g. TensorImage etc.).
   * @param fileWriter The writer supplied by an extension.
   * @param data The data to save in a specific format.
   * @param aFileName The filename.
   * @param propFileName Proposed file name?
   * @return false if writing attempt  failed, true otherwise
   */
  static bool SaveToFileWriter(mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data, const std::string fileName);

private:

  struct Impl;

};

#endif // _QmitkIOUtil__h_
