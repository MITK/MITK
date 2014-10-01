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

#include "mitkCustomMimeType.h"

namespace mitk {

AbstractFileIO::Options AbstractFileIO::GetReaderOptions() const
{
  return AbstractFileReader::GetOptions();
}

us::Any AbstractFileIO::GetReaderOption(const std::string& name) const
{
  return AbstractFileReader::GetOption(name);
}

void AbstractFileIO::SetReaderOptions(const AbstractFileIO::Options& options)
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

AbstractFileIO::AbstractFileIO(const AbstractFileIO& other)
  : AbstractFileIOReader(other)
  , AbstractFileIOWriter(other)
{
}

AbstractFileIO::AbstractFileIO(const std::string& baseDataType)
  : AbstractFileIOReader()
  , AbstractFileIOWriter(baseDataType)
{
}

AbstractFileIO::AbstractFileIO(const std::string& baseDataType,
                               const CustomMimeType& mimeType,
                               const std::string& description)
  : AbstractFileIOReader(mimeType, description)
  , AbstractFileIOWriter(baseDataType, mimeType, description)
{
}

AbstractFileIO::AbstractFileIO(const std::string& baseDataType, const std::string& extension, const std::string& description)
  : AbstractFileIOReader(extension, description)
  , AbstractFileIOWriter(baseDataType, extension, description)
{
}

void AbstractFileIO::SetMimeType(const CustomMimeType& mimeType)
{
  this->AbstractFileReader::SetMimeType(mimeType);
  this->AbstractFileWriter::SetMimeType(mimeType);
}

CustomMimeType AbstractFileIO::GetMimeType() const
{
  CustomMimeType mimeType = this->AbstractFileReader::GetMimeType();
  if (mimeType.GetName() != this->AbstractFileWriter::GetMimeType().GetName())
  {
    MITK_WARN << "Reader and writer mime-tpyes are different, using the mime-type from IFileReader";
  }
  return mimeType;
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

void AbstractFileIO::SetDefaultReaderOptions(const AbstractFileIO::Options& defaultOptions)
{
  this->AbstractFileReader::SetDefaultOptions(defaultOptions);
}

AbstractFileIO::Options AbstractFileIO::GetDefaultReaderOptions() const
{
  return this->AbstractFileReader::GetDefaultOptions();
}

void AbstractFileIO::SetDefaultWriterOptions(const AbstractFileIO::Options& defaultOptions)
{
  this->AbstractFileWriter::SetDefaultOptions(defaultOptions);
}

AbstractFileIO::Options AbstractFileIO::GetDefaultWriterOptions() const
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

IFileReader* AbstractFileIO::ReaderClone() const
{
  return this->IOClone();
}

IFileWriter* AbstractFileIO::WriterClone() const
{
  return this->IOClone();
}

}
