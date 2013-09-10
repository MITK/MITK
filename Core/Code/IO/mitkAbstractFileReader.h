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
#include <MitkExports.h>

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

  /**
   * \brief Returns the service ranking for this file reader.
   *
   * Default is zero and should only be chosen differently for a reason.
   * The priority is used to determine which reader to use if several
   * equivalent readers have been found.
   * It may be used to replace a default reader from MITK in your own project.
   * E.g. if you want to use your own reader for nrrd files instead of the default,
   * implement it and give it a higher priority than zero.
   */
  virtual int GetPriority() const;

  /**
   * @brief Returns the mime-type this reader can handle.
   * @return
   */
  virtual std::string GetMimeType() const;

  /**
  * \brief Returns the file extensions that this FileReader is able to handle.
  *
  * File extensions must not contain a leading period, e.g "nrrd" is correct
  * while "*.nrrd" and ".nrrd" are incorrect.
  */
  virtual std::vector<std::string> GetExtensions() const;

  /**
  * \brief Returns a human readable description of this file reader.
  *
  * This can be used in FileDialogs for example.
  */
  virtual std::string GetDescription() const;

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

  us::ServiceRegistration<IFileReader> RegisterService(us::ModuleContext* context = us::GetModuleContext());

protected:

  AbstractFileReader();
  ~AbstractFileReader();

  AbstractFileReader(const AbstractFileReader& other);

  AbstractFileReader(const std::string& mimeType, const std::string& extension, const std::string& description);

  virtual us::ServiceProperties GetServiceProperties() const;
  virtual us::ServiceProperties GetMimeTypeServiceProperties() const;

  virtual us::ServiceRegistration<IMimeType> RegisterMimeType(us::ModuleContext* context);

  void SetMimeType(const std::string& mimeType);
  void SetCategory(const std::string& category);
  void SetExtensions(const std::vector<std::string>& extensions);
  void SetDescription(const std::string& description);
  void SetPriority(int priority);

private:

  AbstractFileReader& operator=(const AbstractFileReader& other);

  virtual mitk::IFileReader* Clone() const = 0;

  class Impl;
  std::auto_ptr<Impl> d;
};

} // namespace mitk

#endif /* AbstractFileReader_H_HEADER_INCLUDED_C1E7E521 */
