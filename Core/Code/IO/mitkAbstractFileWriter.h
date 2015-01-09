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

#ifndef AbstractFileWriter_H_HEADER_INCLUDED_C1E7E521
#define AbstractFileWriter_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkCoreExports.h>

// MITK
#include <mitkIFileWriter.h>
#include <mitkMimeType.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usGetModuleContext.h>

#include <memory>

namespace us {
  struct PrototypeServiceFactory;
}

namespace mitk {

class CustomMimeType;

/**
 * @brief Base class for writing mitk::BaseData objects to files or streams.
 *
 * In general, all file writers should derive from this class, this way it is
 * made sure that the new implementation is
 * exposed to the Microservice-Framework and that is automatically available troughout MITK.
 * The default implementation only requires one Write()
 * method and the Clone() method to be implemented.
 *
 * @ingroup IO
 */
class MITK_CORE_EXPORT AbstractFileWriter : public mitk::IFileWriter
{
public:

  virtual void SetInput(const BaseData* data);
  virtual const BaseData* GetInput() const;

  virtual void SetOutputLocation(const std::string& location);
  virtual std::string GetOutputLocation() const;

  virtual void SetOutputStream(const std::string& location, std::ostream* os);
  virtual std::ostream* GetOutputStream() const;

  /**
   * \brief Write the base data to the specified location or output stream.
   *
   * This method must be implemented for each specific writer. Call
   * GetOutputStream() first and check for a non-null stream to write to.
   * If the output stream is \c NULL, use GetOutputLocation() to write
   * to a local file-system path.
   *
   * If the reader cannot use streams directly, use GetLocalFile() to retrieve
   * a temporary local file name instead.
   *
   * \throws mitk::Exception
   *
   * \see GetLocalFile()
   * \see IFileWriter::Write()
   */
  virtual void Write() = 0;

  virtual ConfidenceLevel GetConfidenceLevel() const;

  MimeType GetRegisteredMimeType() const;

  virtual Options GetOptions() const;
  virtual us::Any GetOption(const std::string &name) const;

  virtual void SetOptions(const Options& options);
  virtual void SetOption(const std::string& name, const us::Any& value);

  virtual void AddProgressCallback(const ProgressCallback& callback);

  virtual void RemoveProgressCallback(const ProgressCallback& callback);

  us::ServiceRegistration<IFileWriter> RegisterService(us::ModuleContext* context = us::GetModuleContext());
  void UnregisterService();

protected:

  /**
   * @brief A local file representation for streams.
   *
   * If a writer can only work with local files, use an instance
   * of this class to get either a temporary file name for writing
   * to the specified output stream or the original output location
   * if no output stream was set.
   */
  class MITK_CORE_EXPORT LocalFile
  {
  public:
    LocalFile(IFileWriter* writer);

    // Writes to the ostream and removes the temporary file
    ~LocalFile();

    // Creates a temporary file for output operations.
    std::string GetFileName();

  private:

    // disabled
    LocalFile();
    LocalFile(const LocalFile&);
    LocalFile& operator=(const LocalFile& other);

    struct Impl;
    std::auto_ptr<Impl> d;
  };

  /**
   * @brief An output stream wrapper.
   *
   * If a writer can only work with output streams, use an instance
   * of this class to either wrap the specified output stream or
   * create a new output stream based on the output location in the
   * file system.
   */
  class OutputStream : public std::ostream
  {
  public:
    OutputStream(IFileWriter* writer, std::ios_base::openmode mode = std::ios_base::trunc | std::ios_base::out);
    ~OutputStream();
  private:
    std::ostream* m_Stream;
  };

  ~AbstractFileWriter();

  AbstractFileWriter(const AbstractFileWriter& other);

  AbstractFileWriter(const std::string& baseDataType);

  AbstractFileWriter(const std::string& baseDataType, const CustomMimeType& mimeType, const std::string& description);

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
   * @return Get the mime-type this writer can handle.
   */
  const CustomMimeType* GetMimeType() const;

  void SetMimeTypePrefix(const std::string& prefix);
  std::string GetMimeTypePrefix() const;

  /**
   * \brief Sets a human readable description of this writer.
   *
   * This will be used in file dialogs for example.
   */
  void SetDescription(const std::string& description);
  std::string GetDescription() const;

  void SetDefaultOptions(const Options& defaultOptions);
  Options GetDefaultOptions() const;

  /**
   * \brief Set the service ranking for this file writer.
   *
   * Default is zero and should only be chosen differently for a reason.
   * The ranking is used to determine which writer to use if several
   * equivalent writers have been found.
   * It may be used to replace a default writer from MITK in your own project.
   * E.g. if you want to use your own writer for nrrd files instead of the default,
   * implement it and give it a higher ranking than zero.
   */
  void SetRanking(int ranking);
  int GetRanking() const;

  /**
   * \brief Sets the name of the mitk::Basedata that this writer is able to handle.
   *
   * The correct value is the one given as the first parameter in the mitkNewMacro of that BaseData derivate.
   * You can also retrieve it by calling <code>GetNameOfClass()</code> on an instance of said data.
   */
  void SetBaseDataType(const std::string& baseDataType);
  virtual std::string GetBaseDataType() const;

  void ValidateOutputLocation() const;

private:

  AbstractFileWriter& operator=(const AbstractFileWriter& other);

  virtual mitk::IFileWriter* Clone() const = 0;

  class Impl;
  std::auto_ptr<Impl> d;
};

} // namespace mitk

#endif /* AbstractFileWriter_H_HEADER_INCLUDED_C1E7E521 */
