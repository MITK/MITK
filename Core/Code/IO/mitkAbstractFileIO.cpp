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

#include "mitkAbstractFileIO.h"

namespace mitk {

AbstractFileIO::ReaderOptions AbstractFileIO::GetReaderOptions() const
{
  return AbstractFileReader::GetOptions();
}

us::Any AbstractFileIO::GetReaderOption(const std::string& name) const
{
  return AbstractFileReader::GetOption(name);
}

void AbstractFileIO::SetReaderOptions(const AbstractFileIO::ReaderOptions& options)
{
  AbstractFileReader::SetOptions(options);
}

void AbstractFileIO::SetReaderOption(const std::string& name, const us::Any& value)
{
  AbstractFileReader::SetOption(name, value);
}

std::pair<us::ServiceRegistration<IFileReader>, us::ServiceRegistration<IFileReader> >
AbstractFileIO::RegisterService(us::ModuleContext* context)
{
  std::pair<us::ServiceRegistration<IFileReader>, us::ServiceRegistration<IFileReader> > result;
  result.first = this->AbstractFileReader::RegisterService(context);
  result.second = this->AbstractFileWriter::RegisterService(context);
  return result;
}

AbstractFileIO::AbstractFileIO()
{
}

AbstractFileIO::AbstractFileIO(const AbstractFileIO& other)
  : AbstractFileReader(other)
  , AbstractFileWriter(other)
{
}

AbstractFileIO::AbstractFileIO(const std::string& baseDataType,
                               const AbstractFileIO::MimeType& mimeType,
                               const std::string& description)
  : AbstractFileReader(mimeType, description)
  , AbstractFileWriter()
{
  this->AbstractFileWriter::SetBaseDataType(baseDataType);
  this->AbstractFileWriter::SetMimeType(mimeType);
}

AbstractFileIO::AbstractFileIO(const std::string& baseDataType, const std::string& extension, const std::string& description)
  : AbstractFileReader(extension, description)
{
  this->AbstractFileWriter::SetBaseDataType(baseDataType);
  this->AbstractFileWriter::AddExtension(extension);
  this->AbstractFileWriter::SetDescription(description);
}

void AbstractFileIO::SetMimeType(const std::string& mimeType)
{
  this->AbstractFileReader::SetMimeType(mimeType);
  this->AbstractFileWriter::SetMimeType(mimeType);
}

std::string AbstractFileIO::GetMimeType() const
{
  std::string mimeType = this->AbstractFileReader::GetMimeType();
  if (mimeType != this->AbstractFileWriter::GetMimeType())
  {
    MITK_WARN << "Reader and writer mime-tpyes are different, using the mime-type from IFileReader";
  }
  return mimeType;
}

void AbstractFileIO::SetCategory(const std::string& category)
{
  this->AbstractFileReader::SetCategory(category);
  this->AbstractFileWriter::SetCategory(category);
}

std::string AbstractFileIO::GetCategory() const
{
  std::string category = this->AbstractFileReader::GetCategory();
  if (category != this->AbstractFileWriter::GetCategory())
  {
    MITK_WARN << "Reader and writer mime-tpyes are different, using the mime-type from IFileReader";
  }
  return category;
}

std::vector<std::string> AbstractFileIO::GetExtensions() const
{
  std::vector<std::string> extensions = this->AbstractFileReader::GetExtensions();
  if (extensions != this->AbstractFileWriter::GetExtensions())
  {
    MITK_WARN << "Reader and writer extensions are different, using extensions from IFileReader";
  }
  return extensions;
}

void AbstractFileIO::AddExtension(const std::string& extension)
{
  this->AbstractFileReader::AddExtension(extension);
  this->AbstractFileWriter::AddExtension(extension);
}

void AbstractFileIO::SetReaderDescription(const std::string& description)
{
  this->AbstractFileReader::SetDescription(description);
}

std::string AbstractFileIO::GetReaderDescription() const
{
  return this->AbstractFileReader::GetDescription();
}

void AbstractFileIO::SetWriterDescription(const std::string& description)
{
  this->AbstractFileWriter::SetDescription(description);
}

std::string AbstractFileIO::GetWriterDescription() const
{
  return this->AbstractFileWriter::GetDescription();
}

void AbstractFileIO::SetDefaultReaderOptions(const AbstractFileIO::ReaderOptions& defaultOptions)
{
  this->AbstractFileReader::SetDefaultOptions(defaultOptions);
}

AbstractFileIO::ReaderOptions AbstractFileIO::GetDefaultReaderOptions() const
{
  return this->AbstractFileReader::GetDefaultOptions();
}

void AbstractFileIO::SetDefaultWriterOptions(const AbstractFileIO::WriterOptions& defaultOptions)
{
  this->AbstractFileWriter::SetDefaultOptions(defaultOptions);
}

AbstractFileIO::WriterOptions AbstractFileIO::GetDefaultWriterOptions() const
{
  return this->AbstractFileWriter::GetDefaultOptions();
}

void AbstractFileIO::SetReaderRanking(int ranking)
{
  this->AbstractFileReader::SetRanking(ranking);
}

int AbstractFileIO::GetReaderRanking() const
{
  return this->AbstractFileReader::GetRanking();
}

void AbstractFileIO::SetWriterRanking(int ranking)
{
  this->AbstractFileWriter::SetRanking(ranking);
}

int AbstractFileIO::GetWriterRanking() const
{
  return this->AbstractFileWriter::GetRanking();
}



}
