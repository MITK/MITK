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


#ifndef MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataPlayerBase.h>
#include "tinyxml.h"


namespace mitk
{

  /**Documentation
  * \brief This class is a NavigationDataPlayer which can play CSV formatted
  * files in sequential order, which means it doesn't care about timestamps and just
  * outputs the navigationdatas in their sequential order.
  *
  * It is thought to interpret custom csv files. To do so please adapt the column
  * numbers of position and orientation in the internal method GetNavigationDataOutOfOneLine().
  *
  * So far only one (the first) tool is read in as required for the hummel protocol measurements.
  *
  * This class can also interpret MITK style csv files (set filetype to NavigationDataCSV), but
  * you can also use the MITK navigation data player class inside the MITK-IGT module which
  * is newer and better maintained.
  *
  * \ingroup IGT
  */
  class NavigationDataCSVSequentialPlayer
    : public NavigationDataPlayerBase
  {
  public:

    mitkClassMacro(NavigationDataCSVSequentialPlayer, NavigationDataPlayerBase);
    itkNewMacro(Self);

    /**
    * \brief sets the file name and path (if XMLString is set, this is neglected)
    */
    void SetFileName(const std::string& _FileName);

    /**
    * \brief returns the file name and path
    */
    itkGetStringMacro(FileName);

    enum Filetype
    {
      NavigationDataCSV, //for csv files from the MITK navigation data player
      ManualLoggingCSV //for custum csv files
    };
    /**
    * \brief Sets the file type. ManualLoggingCSV is default and is thought for your
    *        custom csv files. You can also set it to NavigationDataCSV, then this
    *        player interprets MITK style csv files.
    */
    itkSetMacro(Filetype, Filetype);

    /**
    * \return Returns true if the player reached the end of the file.
    */
    bool IsAtEnd();

    /**
    * \brief Used for pipeline update just to tell the pipeline
    * that we always have to update
    */
    virtual void UpdateOutputInformation();

    int mitk::NavigationDataCSVSequentialPlayer::GetNumberOfSnapshots();

  protected:
    NavigationDataCSVSequentialPlayer();
    virtual ~NavigationDataCSVSequentialPlayer();

    ///
    /// do the work here
    ///
    virtual void GenerateData();

    std::string m_FileName;

    int m_CurrentPos;
    Filetype m_Filetype;

    //member for the navigation datas which were read (only one output is supported at the moment)
    std::vector<mitk::NavigationData::Pointer> m_NavigationDatas;

    std::vector<mitk::NavigationData::Pointer> GetNavigationDatasFromFile(std::string filename);
    std::vector<std::string> GetFileContentLineByLine(std::string filename);
    mitk::NavigationData::Pointer GetNavigationDataOutOfOneLine(std::string line);

    void FillOutputEmpty(int number);
    mitk::NavigationData::Pointer GetEmptyNavigationData();

  };
} // namespace mitk

#endif /* MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_ */
