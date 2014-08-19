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


#ifndef MITKNavigationDataReaderCSV_H_HEADER_INCLUDED_
#define MITKNavigationDataReaderCSV_H_HEADER_INCLUDED_

#include "mitkNavigationDataReaderInterface.h"

namespace mitk {
  /** This class reads csv logged navigation datas from the hard disc and returns
   *  the navigation data set.
   *
   *  Caution: at the moment only one navigation data is supported which means that only
   *  the data of the first navigation tool in the file is read!
   */
  class MitkIGT_EXPORT NavigationDataReaderCSV : public NavigationDataReaderInterface
  {
  public:

    mitkClassMacro(NavigationDataReaderCSV, NavigationDataReaderInterface);
    itkNewMacro(Self);

    /** @return Returns the NavigationDataSet of the first tool in the given file.
     *          Returns an empty NavigationDataSet if the file could not be read.
     */
    virtual mitk::NavigationDataSet::Pointer Read(std::string fileName);

  protected:
    NavigationDataReaderCSV();
    virtual ~NavigationDataReaderCSV();

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
  };
}

#endif // MITKNavigationDataReaderCSV_H_HEADER_INCLUDED_
