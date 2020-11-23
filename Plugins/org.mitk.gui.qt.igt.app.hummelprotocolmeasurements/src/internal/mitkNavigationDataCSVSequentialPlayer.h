/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataPlayerBase.h>

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
     * @brief SetOptions    sets the options for reading out the data out of the correct postions of the file. They need to be set before using the player
     * @param rightHanded   true if the used coordinate System is right handed, false if it is left handed
     * @param seperatorSign symbol that is used to separate the values in the .csv file
     * @param sampleCount   every n-th sample in the file that should be used
     * @param headerRow     true if the .csv file has a header row otherwise false
     * @param xPos          number of the colum in the .csv file for the x-coordinates of the position
     * @param yPos          number of the colum in the .csv file for the y-coordinates of the position
     * @param zPos          number of the colum in the .csv file for the z-coordinates of the position
     * @param useQuats      true if Quaternions are used to construct the orientation, false if Euler Angles are used
     * @param qx            number of the column in the .csv file for the x component of the quaternion
     * @param qy            number of the column in the .csv file for the y component of the quaternion
     * @param qz            number of the column in the .csv file for the z component of the quaternion
     * @param qr            number of the column in the .csv file for the r component of the quaternion
     * @param azimuth       number of the colum in the .csv file for Azimuth (Euler Angles). Set < 0 if angle is not defined
     * @param elevatino     number of the colum in the .csv file for Elevation (Euler Angles) Set < 0 if angle is not defined
     * @param roll          number of the colum in the .csv file for Roll (Euler Angles) Set < 0 if angle is not defined
     * @param eulerInRadiants   true if the Euler Angles in the .csv file are in radiants, false if they are in degrees
     * @param minNumberOfColumns
     */
    void SetOptions(bool rightHanded, char seperatorSign, int sampleCount, bool headerRow, int xPos, int yPos, int zPos, bool useQuats,
                    int qx, int qy, int qz, int qr, int azimuth, int elevatino, int roll, bool eulerInRadiants, int minNumberOfColums);

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
    void UpdateOutputInformation() override;

    int GetNumberOfSnapshots();

  protected:
    NavigationDataCSVSequentialPlayer();
    ~NavigationDataCSVSequentialPlayer() override;

    ///
    /// do the work here
    ///
    void GenerateData() override;

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

    bool m_RightHanded; //true if the used coordinate System is right handed, false if it is left handed
    char m_SeparatorSign; //symbol that is used to separate the values in the .csv file
    int m_SampleCount; //every n-th sample in the file that should be used
    bool m_HeaderRow; //true if the .csv file has a header row otherwise false
    int m_XPos; //number of the colum in the .csv file for the x-coordinates of the position
    int m_YPos; //number of the colum in the .csv file for the y-coordinates of the position
    int m_ZPos; //number of the colum in the .csv file for the z-coordinates of the position
    bool m_UseQuats; //true if Quaternions are used to construct the orientation, false if Euler Angles are used
    int m_Qx; //number of the column in the .csv file for the x component of the quaternion
    int m_Qy; //number of the column in the .csv file for the y component of the quaternion
    int m_Qz; //number of the column in the .csv file for the z component of the quaternion
    int m_Qr; //number of the column in the .csv file for the r component of the quaternion
    int m_Azimuth; //number of the colum in the .csv file for Azimuth (Euler Angles)
    int m_Elevation; //number of the colum in the .csv file for Elevation (Euler Angles)
    int m_Roll; //number of the colum in the .csv file for Roll (Euler Angles)
    bool m_EulersInRadiants; // true if the Euler Angles in the .csv file are in radiants, false if they are in degrees
    int m_MinNumberOfColumns; //minimal number of columns the file has to have (e.g. if you read data from column 8 the file has to have at least 8 columns)
  };
} // namespace mitk

#endif /* MITKNavigationDataCSVSequentialPlayer_H_HEADER_INCLUDED_ */
