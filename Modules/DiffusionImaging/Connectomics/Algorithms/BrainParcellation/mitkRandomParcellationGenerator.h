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

#ifndef mitkRandomParcellationGenerator_h
#define mitkRandomParcellationGenerator_h

#include<itkImage.h>

#include "mitkCommon.h"
#include "MitkConnectomicsExports.h"

// mitk
#include <mitkCostFunctionBase.h>
#include <mitkRegionVoxelCounter.h>

//Itk Iterators
#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>

#include <vector>

//To use pair
#include <utility>

namespace mitk
{

  template <typename TPixel, unsigned int VImageDimension>
  class RandomParcellationGenerator
  {
  public:
    typedef itk::Image< TPixel, VImageDimension > ImageType;

    //Set-Functions
    void SetImage(itk::Image<TPixel, VImageDimension> *);
    void SetNumberNodes(int inputNumberNodes);
    void SetVariablesForMerging(int givenSizeOfSmallestRegion, int desiredNumberOfParcels, int givenSizeOfSmallestRegionBeginning);
    void SetBoolsForMerging(bool mergingWithNumberParcels, bool mergingWithSmallestParcel, bool justMergeSmallParcels);

    //Main Functions

    /** \brief Sets randomly chosen seed voxels (1x1x1 regions) on the segmented image
    * This is done by creating a vector with unique values, which represents the position of a voxel respectively.
    * For this purpose we count all voxels with value one and choose random numbers.
    */
    void GetRandomSeedVoxels();
    /** \brief Add appropriate voxels of the segmented part to a region (just 6-connected neighborhood)
    * A voxel is appropriate if the cost function value is high enough. Several constraints have to be fulfilled for this purpose.
    */
    void ParcelGrowthOverFaces();
    /** \brief Add voxels of the segmented part to an appropriate region (26-connected neighborhood)
    * Checks which voxels still have value one and to which neighborhood-region each one should be added.
    */
    void FillOverEdgeOrVertex();
    /** \brief Add voxels of the segmented part to an appropriate region (no neighbors necessary)
    * Checks which voxels still have value one and calculates the distance to all parcels.
    * Finally each voxel is added to the region with the smallest distance to it.
    */
    void AllocateIsolatedVoxels();
    /** \brief Merge parcels according to a cost function
    * Looks for the parcel with the smallest number of voxels.
    * Then merges it to a neighborhood-parcel such that the cost function value is small enough.
    * The new number of nodes is returned so we can show them on the GUI.
    */
    int MergeParcels();
    /** \brief Changes the values of the nodes, such that no gaps exist and it starts with value 1*/
    void SetAppropriateValues();
    /** \brief Calculates and shows the size (number of voxels) of all regions on the console*/
    void ShowSizeOfRegions();

  protected:
    //Sub-Functions
    /** \brief Gives back the center of mass and -if wanted- the size (number of voxels) of a parcel*/
    std::vector<double> GetCenterOfMass( itk::ImageRegionIterator<ImageType> it_region, int valueOfRegion, bool getSizeOfRegions );
    /** \brief Calculates the distance between two voxels, the position of the first one is given by an index and the position of the second one is given by a vector*/
    double GetDistance( std::vector<double> centerOfMass, typename ImageType::IndexType indexNewVoxel);
    /** \brief Calculates the distance between two voxels, both positions are given by vectors*/
    double GetDistanceVector( std::vector<double> centerOfMass, std::vector<double> indexNewVoxel);
    /** \brief Gives back the smallest value of an int-vector*/
    int SmallestValue (std::vector<int> distance);
    /** \brief Gives back the smallest value of a double-vector*/
    double SmallestValue (std::vector<double> distance);
    /** \brief Extends the region if the chosen voxel lies outside*/
    typename ImageType::RegionType ExtendedRegion(typename ImageType::RegionType chosenRegion, typename ImageType::IndexType indexChosenVoxel);
    /** \brief Extends the region of a parcel such that the second region lies within*/
    typename ImageType::RegionType ExtendedRegionNotNeighbor(typename ImageType::RegionType chosenRegion, typename ImageType::RegionType smallestRegion);
    /** \brief Checks if a number is an element of the vector already*/
    bool IsUnique (int number, std::vector<int> vec);
    /** \brief Calculates the size (number of voxels) of all regions*/
    void GetSizeOfRegions();

    typedef itk::ImageRegion<3> RegionType;
    //Start the algorithm ParcelGrowthOverFaces with m_OddRegions and transfer them to m_EvenRegions or m_InvalidRegions respectively
    std::vector<std::pair<RegionType, int> > m_EvenRegions;
    std::vector<std::pair<RegionType, int> > m_OddRegions;
    std::vector<std::pair<RegionType, int> > m_InvalidRegions;
    std::vector<int> m_SizeOfRegions;
    //For merging; regions that can't be merged any more
    std::vector<int> m_SizeOfFinishedRegions;

    ImageType * m_Image;
    int m_NumberNodes;
    int m_GivenSizeOfSmallestRegion;
    int m_DesiredNumberOfParcels;
    int m_GivenSizeOfSmallestRegionBeginning;
    bool m_MergingWithNumberParcels;
    bool m_MergingWithSmallestParcel;
    bool m_JustMergeSmallParcels;
  };
}

#include "mitkRandomParcellationGenerator.cpp"

#endif /*  mitkRandomParcellationGenerator_h */