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

#ifndef MITKCUSTOMMIMETYPE_H
#define MITKCUSTOMMIMETYPE_H

#include <MitkCoreExports.h>

#include <mitkServiceInterface.h>

#include <string>
#include <vector>

namespace mitk {

class MimeType;

/**
 * @ingroup IO
 * @ingroup MicroServices_Interfaces
 *
 * @brief The CustomMimeType class represents a custom mime-type which
 *        may be registered as a service object.
 *
 * Instances of this class are usually created and registered as a service.
 * They wire files to specific IFileReader instances and provide data format
 * meta-data for selecting compatible IFileWriter instances.
 */
class MITKCORE_EXPORT CustomMimeType
{
public:

  CustomMimeType();
  CustomMimeType(const std::string& name);
  CustomMimeType(const CustomMimeType& other);
  explicit CustomMimeType(const MimeType& other);

  virtual ~CustomMimeType();

  CustomMimeType& operator=(const CustomMimeType& other);
  CustomMimeType& operator=(const MimeType& other);

  std::string GetName() const;
  std::string GetCategory() const;
  std::vector<std::string> GetExtensions() const;
  std::string GetComment() const;

  virtual bool AppliesTo(const std::string& path) const;
  /**
  * \brief Provides the first matching extension
  *
  * Checks whether any of its extensions are present at the end of the provided path.
  * Returns the first found one.
  */
  std::string GetExtension(const std::string& path) const;
  /**
  * \brief Provides the filename minus the extension
  *
  * Checks whether any of its extensions are present at the end of the provided path.
  * Returns the filename without that extension and without the directory.
  */
  std::string GetFilenameWithoutExtension(const std::string& path) const;

  void SetName(const std::string& name);
  void SetCategory(const std::string& category);
  void SetExtension(const std::string& extension);
  void AddExtension(const std::string& extension);
  void SetComment(const std::string& comment);

  void Swap(CustomMimeType& r);

  virtual CustomMimeType* Clone() const;

private:

  // returns true if an extension was found
  bool ParsePathForExtension(const std::string& path, std::string& extension, std::string& filename ) const;

  struct Impl;
  Impl* d;
};

void swap(CustomMimeType& l, CustomMimeType& r);

}

MITK_DECLARE_SERVICE_INTERFACE(mitk::CustomMimeType, "org.mitk.CustomMimeType")

#endif // MITKCUSTOMMIMETYPE_H
