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

#ifndef DOXYGEN_SKIP

// Skip this code during Doxygen processing, because it only
// exists to resolve name clashes when inheriting from both
// AbstractFileReader and AbstractFileWriter.

class AbstractFileIOReader : public AbstractFileReader
{
public:

  virtual ConfidenceLevel GetReaderConfidenceLevel() const
  {
    return AbstractFileReader::GetConfidenceLevel();
  }

  virtual ConfidenceLevel GetConfidenceLevel() const
  {
    return this->GetReaderConfidenceLevel();
  }

protected:

  AbstractFileIOReader() {}

  AbstractFileIOReader(const CustomMimeType& mimeType, const std::string& description)
    : AbstractFileReader(mimeType, description) {}

private:

  virtual IFileReader* ReaderClone() const = 0;
  IFileReader* Clone() const { return ReaderClone(); }
};

struct AbstractFileIOWriter : public AbstractFileWriter
{
  virtual ConfidenceLevel GetWriterConfidenceLevel() const
  {
    return AbstractFileWriter::GetConfidenceLevel();
  }

  virtual ConfidenceLevel GetConfidenceLevel() const
  {
    return this->GetWriterConfidenceLevel();
  }

protected:

  AbstractFileIOWriter(const std::string& baseDataType) : AbstractFileWriter(baseDataType) {}

  AbstractFileIOWriter(const std::string& baseDataType, const CustomMimeType& mimeType,
                       const std::string& description)
    : AbstractFileWriter(baseDataType, mimeType, description) {}

private:

  virtual IFileWriter* WriterClone() const = 0;
  IFileWriter* Clone() const { return WriterClone(); }
};

#endif // DOXYGEN_SKIP

class MITK_CORE_EXPORT AbstractFileIO : public AbstractFileIOReader, public AbstractFileIOWriter
{
public:

  Options GetReaderOptions() const;
  us::Any GetReaderOption(const std::string &name) const;

  void SetReaderOptions(const Options& options);
  void SetReaderOption(const std::string& name, const us::Any& value);

  Options GetWriterOptions() const;
  us::Any GetWriterOption(const std::string &name) const;

  void SetWriterOptions(const Options& options);
  void SetWriterOption(const std::string& name, const us::Any& value);

  virtual ConfidenceLevel GetReaderConfidenceLevel() const;

  virtual ConfidenceLevel GetWriterConfidenceLevel() const;

  std::pair<us::ServiceRegistration<IFileReader>, us::ServiceRegistration<IFileReader> >
  RegisterService(us::ModuleContext* context = us::GetModuleContext());

protected:

  AbstractFileIO(const AbstractFileIO& other);

  AbstractFileIO(const std::string& baseDataType);

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
  explicit AbstractFileIO(const std::string& baseDataType, const CustomMimeType& mimeType,
                          const std::string& description);

  /**
   * Associate this reader with the given file extension.
   *
   * Additonal file extensions can be added by sub-classes by calling AddExtension
   * or SetExtensions on the CustomMimeType object returned by GetMimeType() and
   * setting the modified object again via SetMimeType().
   *
   * @param extension The file extension (without a leading period) for which a registered
   *        mime-type object is looked up and associated with this instance.
   * @param description A human readable description of this reader.
   *
   * @see RegisterService
   */
  explicit AbstractFileIO(const std::string& baseDataType, const std::string& extension,
                          const std::string& description);

  void SetMimeType(const CustomMimeType& mimeType);

  /**
   * @return The mime-type this reader can handle.
   */
  const CustomMimeType* GetMimeType() const;

  void SetReaderDescription(const std::string& description);
  std::string GetReaderDescription() const;

  void SetWriterDescription(const std::string& description);
  std::string GetWriterDescription() const;

  void SetDefaultReaderOptions(const Options& defaultOptions);
  Options GetDefaultReaderOptions() const;

  void SetDefaultWriterOptions(const Options& defaultOptions);
  Options GetDefaultWriterOptions() const;

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

  virtual AbstractFileIO* IOClone() const = 0;

  virtual IFileReader* ReaderClone() const;
  virtual IFileWriter* WriterClone() const;

};

}

#endif // MITKABSTRACTFILEIO_H
