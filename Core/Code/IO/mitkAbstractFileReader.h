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
#include <mitkSimpleMimeType.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usGetModuleContext.h>

namespace us {
  struct PrototypeServiceFactory;
}

namespace mitk {

/**
 * @brief Interface class of readers that read from files
 * @ingroup Process
 */
class MITK_CORE_EXPORT AbstractFileReader : public mitk::IFileReader
{

public:

  virtual std::vector<itk::SmartPointer<BaseData> > Read(const std::string& path);

  virtual std::vector<itk::SmartPointer<BaseData> > Read(std::istream& stream) = 0;

  virtual std::vector<std::pair<itk::SmartPointer<BaseData>,bool> > Read(const std::string& path, mitk::DataStorage& ds);

  virtual std::vector<std::pair<itk::SmartPointer<BaseData>,bool> > Read(std::istream& stream, mitk::DataStorage& ds);

  virtual OptionList GetOptions() const;

  virtual void SetOptions(const OptionList& options);

  /**
   * @brief Checks if the specified path can be read.
   *
   * The default implementation checks if the path exists and contains a
   * file extension associated with the mime-type of this reader.
   * It then creates a std::ifstream object for the given path and
   * calls CanRead(const std::istream&).
   *
   * @param path The absolute path to a file.
   * @return \c true if the file can be read, \c false otherwise.
   */
  virtual bool CanRead(const std::string& path) const;

  /**
   * @brief Checks if the specified input stream can be read.
   *
   * @param stream The stream to be read.
   * @return \c true if the stream is good, \c false otherwise.
   */
  virtual bool CanRead(std::istream& stream) const;

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
   *        IMimeType object is looked up and associated with this reader instance.
   * @param description A human readable description of this reader.
   */
  us::ServiceRegistration<IFileReader> RegisterService(us::ModuleContext* context = us::GetModuleContext());

protected:

  class MITK_CORE_EXPORT MimeType : public std::string
  {
  public:
    MimeType(const std::string& mimeType);

  private:
    MimeType();

    friend class AbstractFileReader;
  };

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
  explicit AbstractFileReader(const MimeType& mimeType, const std::string& description);

  /**
   * Associate this reader with the given file extension.
   *
   * Additonal file extensions can be added by sub-classes by calling AddExtension
   * or SetExtensions.
   *
   * @param extension The file extension (without a leading period) for which a registered
   *        IMimeType object is looked up and associated with this reader instance.
   * @param description A human readable description of this reader.
   *
   * @see RegisterService
   */
  explicit AbstractFileReader(const std::string& extension, const std::string& description);

  virtual us::ServiceProperties GetServiceProperties() const;

  /**
   * @brief Returns the mime-type this reader can handle.
   * @return
   */
  std::string GetMimeType() const;

  /**
   * Registers a new IMimeType service object.
   *
   * This method is called from RegisterService and the default implementation
   * registers a new IMimeType service object if all of the following conditions
   * are true:
   *
   *  - The reader
   *
   * @param context
   * @return
   * @throws std::invalid_argument if \c context is NULL.
   */
  virtual us::ServiceRegistration<IMimeType> RegisterMimeType(us::ModuleContext* context);

  void SetMimeType(const std::string& mimeType);
  void SetCategory(const std::string& category);
  void AddExtension(const std::string& extension);
  void SetDescription(const std::string& description);

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

  std::string GetCategory() const;
  std::vector<std::string> GetExtensions() const;
  std::string GetDescription() const;

private:

  AbstractFileReader& operator=(const AbstractFileReader& other);

  virtual mitk::IFileReader* Clone() const = 0;

  class Impl;
  std::auto_ptr<Impl> d;
};

} // namespace mitk

#endif /* AbstractFileReader_H_HEADER_INCLUDED_C1E7E521 */
