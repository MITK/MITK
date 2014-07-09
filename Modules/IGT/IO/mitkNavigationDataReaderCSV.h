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

    std::vector<std::string> GetFileContentLineByLine(std::string filename);
    mitk::NavigationData::Pointer GetNavigationDataOutOfOneLine(std::string line);
    mitk::NavigationData::Pointer GetEmptyNavigationData();

  };
}

#endif // MITKNavigationDataReaderCSV_H_HEADER_INCLUDED_
