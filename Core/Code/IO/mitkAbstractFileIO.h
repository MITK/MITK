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

#ifndef MITKABSTRACTFILEIO_H
#define MITKABSTRACTFILEIO_H

#include "mitkAbstractFileReader.h"
#include "mitkAbstractFileWriter.h"

namespace mitk {

class MITK_CORE_EXPORT AbstractFileIO : public AbstractFileReader, public AbstractFileWriter
{
public:

  typedef IFileReader::Options ReaderOptions;
  typedef IFileWriter::Options WriterOptions;

  ReaderOptions GetReaderOptions() const;
  us::Any GetReaderOption(const std::string &name) const;

  void SetReaderOptions(const ReaderOptions& options);
  void SetReaderOption(const std::string& name, const us::Any& value);

  std::pair<us::ServiceRegistration<IFileReader>, us::ServiceRegistration<IFileReader> >
  RegisterService(us::ModuleContext* context = us::GetModuleContext());

protected:

  typedef AbstractFileReader::MimeType MimeType;

  AbstractFileIO();

  AbstractFileIO(const AbstractFileIO& other);

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
  explicit AbstractFileIO(const std::string& baseDataType, const MimeType& mimeType,
                          const std::string& description);

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
  explicit AbstractFileIO(const std::string& baseDataType, const std::string& extension,
                          const std::string& description);

  void SetMimeType(const std::string& mimeType);

  /**
   * @return The mime-type this reader can handle.
   */
  std::string GetMimeType() const;

  void SetCategory(const std::string& category);
  std::string GetCategory() const;

  std::vector<std::string> GetExtensions() const;
  void AddExtension(const std::string& extension);

  void SetReaderDescription(const std::string& description);
  std::string GetReaderDescription() const;

  void SetWriterDescription(const std::string& description);
  std::string GetWriterDescription() const;

  void SetDefaultReaderOptions(const ReaderOptions& defaultOptions);
  ReaderOptions GetDefaultReaderOptions() const;

  void SetDefaultWriterOptions(const WriterOptions& defaultOptions);
  WriterOptions GetDefaultWriterOptions() const;

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
  void SetReaderRanking(int ranking);
  int GetReaderRanking() const;

  void SetWriterRanking(int ranking);
  int GetWriterRanking() const;

private:

  AbstractFileIO& operator=(const AbstractFileIO& other);

  virtual AbstractFileIO* Clone() const = 0;

};

}

#endif // MITKABSTRACTFILEIO_H
