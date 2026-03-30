/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStlVolumeTimeSeriesReader_h
#define mitkStlVolumeTimeSeriesReader_h

#include <mitkCommon.h>
#include <mitkFileSeriesReader.h>
#include <mitkSurfaceSource.h>

namespace mitk
{
  /**
   * \brief Reader for a series of volume files in STL format.
   *
   * Reads multiple STL files as time steps of a single surface time series.
   */
  class StlVolumeTimeSeriesReader : public SurfaceSource, public FileSeriesReader
  {
  public:
    mitkClassMacro(StlVolumeTimeSeriesReader, FileSeriesReader);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    void GenerateData() override;

    StlVolumeTimeSeriesReader();

    ~StlVolumeTimeSeriesReader() override;

    /** \brief Time when the header was last read. */
    itk::TimeStamp m_ReadHeaderTime;
  };

} // namespace mitk

#endif
