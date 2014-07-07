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

#ifndef MITKFILEREADERWRITERBASE_H
#define MITKFILEREADERWRITERBASE_H

#include <mitkMessage.h>
#include <mitkSimpleMimeType.h>

#include <usAny.h>
#include <usServiceRegistration.h>
#include <usModuleContext.h>

namespace mitk {

class FileReaderWriterBase
{
public:

  typedef std::map<std::string,us::Any> Options;
  typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

  FileReaderWriterBase();

  Options GetOptions() const;
  us::Any GetOption(const std::string &name) const;

  void SetOptions(const Options& options);
  void SetOption(const std::string& name, const us::Any& value);

  void SetDefaultOptions(const Options& defaultOptions);
  Options GetDefaultOptions() const;

  /**
   * \brief Set the service ranking for this file reader.
   *
   * Default is zero and should only be chosen differently for a reason.
   * The ranking is used to determine which reader to use if several
   * equivalent readers have been found.
   * It may be used to replace a default reader from MITK in your own project.
   * E.g. if you want to use your own reader for nrrd files instead of the default,
   * implement it and give it a higher ranking than zero.
   */
  void SetRanking(int ranking);
  int GetRanking() const;

  void SetMimeType(const std::string& mimeType);
  std::string GetMimeType() const;

  void AddExtension(const std::string& extension);
  std::vector<std::string> GetExtensions() const;
  bool HasExtension(const std::string& extension);

  void SetDescription(const std::string& description);
  std::string GetDescription() const;

  void SetCategory(const std::string& category);
  std::string GetCategory() const;

  void AddProgressCallback(const ProgressCallback& callback);
  void RemoveProgressCallback(const ProgressCallback& callback);

  us::ServiceRegistration<IMimeType> RegisterMimeType(us::ModuleContext* context);
  void UnregisterMimeType();

protected:

  FileReaderWriterBase(const FileReaderWriterBase& other);

  std::string m_MimeType;
  std::string m_Category;
  std::vector<std::string> m_Extensions;
  std::string m_Description;
  int m_Ranking;

  /**
   * \brief Options supported by this reader. Set sensible default values!
   *
   * Can be left emtpy if no special options are required.
   */
  Options m_Options;

  Options m_DefaultOptions;

  //us::PrototypeServiceFactory* m_PrototypeFactory;

  Message1<float> m_ProgressMessage;

  SimpleMimeType m_SimpleMimeType;
  us::ServiceRegistration<IMimeType> m_MimeTypeReg;

private:

  // purposely not implemented
  FileReaderWriterBase& operator=(const FileReaderWriterBase& other);

};

}

#endif // MITKFILEREADERWRITERBASE_H
