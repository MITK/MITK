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

#ifndef AbstractFileReader_H_HEADER_INCLUDED_C1E7E521
#define AbstractFileReader_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkCoreExports.h>

// MITK
#include <mitkIFileReader.h>
#include <mitkBaseData.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usGetModuleContext.h>

namespace us {
  struct PrototypeServiceFactory;
}

namespace mitk {

class CustomMimeType;

/**
 * @brief Base class for creating mitk::BaseData objects from files or streams.
 * @ingroup Process
 */
class MITK_CORE_EXPORT AbstractFileReader : public mitk::IFileReader
{

public:

  /**
   * @brief Reads the given \c path and creates a list BaseData objects.
   *
   * The default implementation opens a std::ifstream in binary mode for reading
   * and passed the stream to Read(std::istream&).
   *
   * @param path The absolute path to a file include the file name extension.
   * @return
   */
  virtual std::vector<itk::SmartPointer<BaseData> > Read(const std::string& path);

  virtual std::vector<itk::SmartPointer<BaseData> > Read(std::istream& stream) = 0;

  virtual DataStorage::SetOfObjects::Pointer Read(const std::string& path, mitk::DataStorage& ds);

  virtual DataStorage::SetOfObjects::Pointer Read(std::istream& stream, mitk::DataStorage& ds);

  virtual ConfidenceLevel GetConfidenceLevel(const std::string &path) const;

  virtual Options GetOptions() const;
  virtual us::Any GetOption(const std::string &name) const;

  virtual void SetOptions(const Options& options);
  virtual void SetOption(const std::string& name, const us::Any& value);

  virtual void AddProgressCallback(const ProgressCallback& callback);

  virtual void RemoveProgressCallback(const ProgressCallback& callback);

  /**
   * Associate this reader with the MIME type returned by the current IMimeTypeProvider
   * service for the provided extension if the MIME type exists, otherwise registers
   * a new MIME type when RegisterService() is called.
   *
   * If no MIME type for \c extension is already registered, a call to RegisterService()
   * will register a new MIME type and associate this reader instance with it. The MIME
   * type id can be set via SetMimeType() or it will be auto-generated using \c extension,
   * having the form "application/vnd.mitk.<extension>".
   *
   * @param extension The file extension (without a leading period) for which a registered
   *        mime-type object is looked up and associated with this reader instance.
   * @param description A human readable description of this reader.
   */
  us::ServiceRegistration<IFileReader> RegisterService(us::ModuleContext* context = us::GetModuleContext());
  void UnregisterService();

protected:

  AbstractFileReader();
  ~AbstractFileReader();

  AbstractFileReader(const AbstractFileReader& other);

  /**
   * Associate this reader instance with the given MIME type.
   *
   * @param mimeType The mime type this reader can read.
   * @param description A human readable description of this reader.
   *
   * @throws std::invalid_argument if \c mimeType is empty.
   *
   * @see RegisterService
   */
  explicit AbstractFileReader(const CustomMimeType& mimeType, const std::string& description);

  /**
   * Associate this reader with the given file extension.
   *
   * Additonal file extensions can be added by sub-classes by calling AddExtension
   * or SetExtensions.
   *
   * @param extension The file extension (without a leading period) for which a registered
   *        mime-type object is looked up and associated with this reader instance.
   * @param description A human readable description of this reader.
   *
   * @see RegisterService
   */
  explicit AbstractFileReader(const std::string& extension, const std::string& description);

  virtual us::ServiceProperties GetServiceProperties() const;

  /**
   * Registers a new CustomMimeType service object.
   *
   * This method is called from RegisterService and the default implementation
   * registers a new mime-type service object if all of the following conditions
   * are true:
   *
   *  - TODO
   *
   * @param context
   * @return
   * @throws std::invalid_argument if \c context is NULL.
   */
  virtual us::ServiceRegistration<CustomMimeType> RegisterMimeType(us::ModuleContext* context);

  void SetMimeType(const CustomMimeType& mimeType);

  /**
   * @return The mime-type this reader can handle.
   */
  CustomMimeType GetMimeType() const;

  void SetDescription(const std::string& description);
  std::string GetDescription() const;

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

  virtual void SetDefaultDataNodeProperties(DataNode* node, const std::string& filePath);

private:

  AbstractFileReader& operator=(const AbstractFileReader& other);

  virtual mitk::IFileReader* Clone() const = 0;

  class Impl;
  std::auto_ptr<Impl> d;
};

} // namespace mitk

#endif /* AbstractFileReader_H_HEADER_INCLUDED_C1E7E521 */
