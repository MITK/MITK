/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-02-10 18:08:54 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16228 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataPlayerBase.h>
#include "tinyxml.h"


namespace mitk
{

  /**Documentation
  * \brief This class is a NavigationDataPlayer which can play CSV formatted
  * files in sequential order, which means it doesn't
  * care about timestamps and just
  * outputs the navigationdatas in their sequential order
  *
  * \ingroup IGT
  */
  class NavigationDataCSVSequentialPlayer
    : public NavigationDataPlayerBase
  {
  public:

    enum Filetype
      {
      NavigationDataCSV,
      ManualLoggingCSV
      };

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

    itkSetMacro(Filetype,Filetype);
  
    bool IsAtEnd();

    /**
    * \brief Used for pipeline update just to tell the pipeline
    * that we always have to update
    */
    virtual void UpdateOutputInformation();

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