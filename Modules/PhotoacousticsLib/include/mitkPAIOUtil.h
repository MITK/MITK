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

#ifndef MITKPHOTOACOUSTICIO_H
#define MITKPHOTOACOUSTICIO_H

#include "MitkPhotoacousticsLibExports.h"

#include <map>
#include <mutex>
#include <thread>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

#include "mitkPAVolume.h"
#include <mitkPAInSilicoTissueVolume.h>
#include <mitkPAFluenceYOffsetPair.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT IOUtil final
    {
    public:

      struct Position
      {
        int x, z;

        bool operator <(const Position& rhs) const
        {
          return (x < rhs.x || ((x == rhs.x) && z < rhs.z));
        }
      };

      /**
       * @brief LoadFluenceContributionMaps loads multiple nrrd files from a given folder
       * and sorts them into a position map if such meta information is available in
       * the filename. </br>
       * As this method was specifically designed to load many fluence files, the
       * naming convention for the position values is: </br>
       *
       * <b>*_pN,N,NFluence*.nrrd</b>
       *
       * @param foldername
       * @param blur
       * @param progress
       *
       * @return
       */
      static std::map<Position, Volume::Pointer> LoadFluenceContributionMaps(
        std::string foldername, double blur, int* progress, bool log10 = false);

      /**
       * @brief loads a nrrd file from a given filename.
       * @param filename
       * @param sigma
       * @return nullptr if the given filename was invalid.
       */
      static Volume::Pointer LoadNrrd(std::string filename, double sigma = 0);

      /**
      *@brief returns the number of .nrrd files in a given directory
      */
      static int GetNumberOfNrrdFilesInDirectory(std::string directory);

      /**
      *@brief returns a list of all .nrrd files in a given directory
      */
      static std::vector<std::string> GetListOfAllNrrdFilesInDirectory(
        std::string directory, bool keepFileFormat = false);

      /**
      *@brief convenience method to check wether a given strings ends on a given substring.
      *@return false if the substring is longer than the string or if the strings are empty.
      */
      static bool DoesFileHaveEnding(std::string const &fullString, std::string const &ending);

      /**
      *@brief returns all child folders from a folder if there is no .nrrd file in the folder
      * If there IS a .nrrd file in the folder this method will return the given folder path.
      */
      static std::vector<std::string> GetAllChildfoldersFromFolder(std::string folderPath);

      /**
      *@brief loads an in silico tissue volume from a saved nrrd file.
      */
      static InSilicoTissueVolume::Pointer LoadInSilicoTissueVolumeFromNrrdFile(std::string filePath);

      /**
       * @brief LoadFluenceSimulation
       * Adds a MC Simulation nrrd file to this composed volume.
       * The given file needs to contain the meta information "y-offset".
       * This method ensures that all added slices are in the correct order
       * corresponding to their y-offset.
       *
       * @param nrrdFile path to the nrrd file on hard drive
       */
      static FluenceYOffsetPair::Pointer LoadFluenceSimulation(std::string fluenceSimulation);

    private:

      IOUtil();
      ~IOUtil();
    };
  }
}
#endif // MITKPHOTOACOUSTICIO_H
