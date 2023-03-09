/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNavigationDataReaderCSV_h
#define mitkNavigationDataReaderCSV_h

#include <MitkIGTIOExports.h>

#include <mitkAbstractFileReader.h>
#include <mitkNavigationDataSet.h>

namespace mitk {
  /** This class reads csv logged navigation datas from the hard disc and returns
   *  the navigation data set.
   *
   *  Caution: at the moment only one navigation data is supported which means that only
   *  the data of the first navigation tool in the file is read!
   */
  class MITKIGTIO_EXPORT NavigationDataReaderCSV : public AbstractFileReader
  {
  public:

    NavigationDataReaderCSV();
    ~NavigationDataReaderCSV() override;

    /** @return Returns the NavigationDataSet of the first tool in the given file.
     *          Returns an empty NavigationDataSet if the file could not be read.
     */
    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    /**
     * /brief Creates a NavigationData Pointer based on the given Input.
     */
    mitk::NavigationData::Pointer CreateNd(std::string timestamp, std::string valid, std::string X, std::string Y, std::string Z, std::string QX, std::string QY, std::string QZ, std::string QR);

    /**
     * /brief Presents File Content line by line
     */
    std::vector<std::string> GetFileContentLineByLine(std::string filename);

    /**
     * /brief Calculates the Number of Tools based on the number of colums per line.
     */
    int getNumberOfToolsInLine(std::string line);

    /**
     * /brief Converts string to double returns zero if failing
     */
    std::vector<mitk::NavigationData::Pointer> parseLine(std::string line, int NumOfTools);

    /**
     * /brief Converts string to double returns zero if failing
     */
    double StringToDouble( const std::string& s );

    /**
     * /brief Split line in elemens based on a given delim
     */
    std::vector<std::string> splitLine(std::string line);

    NavigationDataReaderCSV(const NavigationDataReaderCSV& other);

    mitk::NavigationDataReaderCSV* Clone() const override;

  };
}

#endif
