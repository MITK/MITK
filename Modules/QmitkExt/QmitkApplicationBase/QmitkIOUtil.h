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

#include "QmitkExtExports.h"
#include <mitkCommon.h>

// std
#include <string>

// mitk includes
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include "itkImage.h"
#include <mitkFileWriterWithInformation.h>

#include <qstring.h>


namespace mitk
{

class QmitkExt_EXPORT QmitkIOUtil
{
    /**
     *
     *
     *
    */
public:

  // internal vessel graph save code removed
  static void SaveBaseDataWithDialog(BaseData *data, std::string path);

  static void SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName = "");

  static void SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName = "");

  static void SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName = "");

//  SaveDataNode
//  LoadPointSet
//  LoadImage
//  LoadSurface
//  LoadDataNode
//  LoadBaseData

protected:

  /**
   * @brief GetFileNameWithQDialog Opens a QDialog and returns the filename.
   * @param caption Caption for the QDialog.
   * @param defaultFilename Default filename (e.g. "NewImage.nrrd" for images).
   * @param filter Filters the data according to data types (e.g. *.nrrd; *.png; etc. for images).
   * @param selectedFilter Default selected filter for the data.
   * @return The file name as QString.
   */
  static QString GetFileNameWithQDialog(QString caption, QString defaultFilename, QString filter, QString* selectedFilter = 0);

  static void SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data, const char* aFileName, const char* propFileName);

};
} //end namespace mitk
#endif // _QmitkIOUtil__h_
