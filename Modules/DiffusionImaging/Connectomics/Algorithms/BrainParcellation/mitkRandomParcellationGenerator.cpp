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

//#include <mitkRandomParcellationGenerator.h>

//To sort
#include <algorithm>

//To use sqrt
#include <cmath>

//Itk Iterators
#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>

//for the use of abs()
#include <cstdlib>

//Better Random Function xrand (extended range)
#define XRAND_MAX (RAND_MAX*(RAND_MAX + 2))
unsigned int xrand (void)
{
  return rand () * (RAND_MAX + 1) + rand ();
}

//Definition of the Set-Functions
template <typename TPixel, unsigned int VImageDimension>
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::SetImage(itk::Image<TPixel, VImageDimension> * image)
{
  m_Image = image;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::SetNumberNodes(int numberNodes)
{
  m_NumberNodes = numberNodes;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::SetVariablesForMerging (int givenSizeOfSmallestRegion, int desiredNumberOfParcels, int givenSizeOfSmallestRegionBeginning)
{
  m_GivenSizeOfSmallestRegion = givenSizeOfSmallestRegion;
  m_DesiredNumberOfParcels = desiredNumberOfParcels;
  m_GivenSizeOfSmallestRegionBeginning = givenSizeOfSmallestRegionBeginning;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::SetBoolsForMerging(bool mergingWithNumberParcels, bool mergingWithSmallestParcel, bool justMergeSmallParcels)
{
  m_MergingWithNumberParcels = mergingWithNumberParcels;
  m_MergingWithSmallestParcel = mergingWithSmallestParcel;
  m_JustMergeSmallParcels = justMergeSmallParcels;
}

//Definition of other functions

//Calculates the Center of mass (COM) and m_SizeOfRegions
template <typename TPixel, unsigned int VImageDimension>
std::vector<double> mitk::RandomParcellationGenerator<TPixel, VImageDimension>::GetCenterOfMass( itk::ImageRegionIterator<ImageType> it_region, int valueOfRegion, bool getSizeOfRegions)
{
  //Counts all tagged voxels in this region

  typedef itk::Image< TPixel, VImageDimension > ImageType;
  int currentSizeOfRegion (0);
  std::vector<typename ImageType::IndexType> indexVoxel;
  std::vector<double> centerOfMass;
  double xValue(0);
  double yValue(0);
  double zValue(0);

  for (it_region.GoToBegin(); !it_region.IsAtEnd(); ++it_region)
  {
    if(it_region.Value() == valueOfRegion)
    {
      indexVoxel.push_back(it_region.GetIndex());
      currentSizeOfRegion++;
    }
  }

  if (getSizeOfRegions == true)
  {
    m_SizeOfRegions.push_back(currentSizeOfRegion);
  }

  xValue = 0;
  yValue = 0;
  zValue = 0;

  for (int i = 0; i < currentSizeOfRegion; i++)
  {
    xValue += indexVoxel[i][0];
    yValue += indexVoxel[i][1];
    zValue += indexVoxel[i][2];
  }

  centerOfMass.push_back(xValue/currentSizeOfRegion);
  centerOfMass.push_back(yValue/currentSizeOfRegion);
  centerOfMass.push_back(zValue/currentSizeOfRegion);

  return centerOfMass;
}

template <typename TPixel, unsigned int VImageDimension>
double mitk::RandomParcellationGenerator<TPixel, VImageDimension>::GetDistance( std::vector<double> centerOfMass, typename itk::Image< TPixel, VImageDimension >::IndexType indexNewVoxel)
{
  double distancetemp(0);

  for (int i = 0; i < 3; i++)
  {
    distancetemp += (centerOfMass[i] - indexNewVoxel[i])*(centerOfMass[i] - indexNewVoxel[i]);
  }
  return sqrt(distancetemp);
}

template <typename TPixel, unsigned int VImageDimension>
double mitk::RandomParcellationGenerator<TPixel, VImageDimension>::GetDistanceVector( std::vector<double> centerOfMass, std::vector<double> indexNewVoxel)
{
  double distancetemp(0);

  for (int i = 0; i < 3; i++)
  {
    distancetemp += (centerOfMass[i] - indexNewVoxel[i])*(centerOfMass[i] - indexNewVoxel[i]);
  }
  return sqrt(distancetemp);
}


template < typename TPixel, unsigned int VImageDimension >
int mitk::RandomParcellationGenerator<TPixel, VImageDimension>::SmallestValue (std::vector<int> distance)
{
  int distancemin;
  distancemin = distance[0];

  for (int i = 1; i < distance.size(); i++)
  {
    if (distance[i] < distancemin)
    {
      distancemin = distance[i];
    }
  }
  return distancemin;
}

template < typename TPixel, unsigned int VImageDimension >
double mitk::RandomParcellationGenerator<TPixel, VImageDimension>::SmallestValue (std::vector<double> distance)
{
  double distancemin;
  distancemin = distance[0];

  for (int i = 1; i < distance.size(); i++)
  {
    if (distance[i] < distancemin)
    {
      distancemin = distance[i];
    }
  }
  return distancemin;
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::SetAppropriateValues()
{
  typedef itk::Image< TPixel, VImageDimension >         TemplateImageType;

  itk::ImageRegionIterator<TemplateImageType> it_image(m_Image, m_Image->GetLargestPossibleRegion());

  TPixel minimumValue, maximumValue;

  it_image.GoToBegin();
  maximumValue = minimumValue = it_image.Value();

  for(it_image.GoToBegin(); !it_image.IsAtEnd(); ++it_image)
  {
    if ( it_image.Value() < minimumValue )
    {
      minimumValue = it_image.Value();
    }
    else
    {
      if ( it_image.Value() > maximumValue )
      {
        maximumValue = it_image.Value();
      }
    }
  }

  int range = int ( maximumValue - minimumValue ); //needs to be castable to int
  int offset = int ( minimumValue );

  if ( range < 0 ) //error
  {
    return;
  }

  std::vector< unsigned int > histogram;
  histogram.resize( range + 1, 0 );

  for(it_image.GoToBegin(); !it_image.IsAtEnd(); ++it_image)
  {
    histogram[ int ( it_image.Value() ) - offset ] += 1;
  }

  int gapCounter = 0; //this variable will be used to count the empty labels

  //stores how much has to be subtracted from the image to remove gaps
  std::vector< TPixel > subtractionStorage;
  subtractionStorage.resize( range + 1, 0 );

  for( int index = 0; index <= range; index++ )
  {
    if( histogram[ index ] == 0 )
    {
      gapCounter++; //if the label is empty, increase gapCounter
    }
    else
    {
      subtractionStorage[ index ] = TPixel ( gapCounter );
    }
  }

  //remove gaps from label image
  for(it_image.GoToBegin(); !it_image.IsAtEnd(); ++it_image)
  {
    it_image.Value() = it_image.Value() - subtractionStorage[ int ( it_image.Value() ) ];
  }
}



template < typename TPixel, unsigned int VImageDimension >
typename itk::Image< TPixel, VImageDimension >::RegionType mitk::RandomParcellationGenerator<TPixel, VImageDimension>::ExtendedRegion(typename itk::Image< TPixel, VImageDimension >::RegionType chosenRegion, typename ImageType::IndexType indexChosenVoxel)
{
  itk::Size<3> size = chosenRegion.GetSize();
  itk::Index<3> start = chosenRegion.GetIndex();
  std::vector<int> greatestValues;
  std::vector<int> smallestValues;

  for(int i = 0; i < 3; i++)
  {
    greatestValues.push_back(start[i] + size[i]);
    smallestValues.push_back(start[i]);
  }

  for(int i = 0; i < 3; i++)
  {
    if (indexChosenVoxel[i] == greatestValues[i])
    {
      size[i] += 1;
    }
    if (indexChosenVoxel[i] == smallestValues[i] - 1)
    {
      start[i] -= 1;
      size[i] += 1;
    }
  }

  chosenRegion.SetSize(size);
  chosenRegion.SetIndex(start);

  return chosenRegion;
}

template < typename TPixel, unsigned int VImageDimension >
typename itk::Image< TPixel, VImageDimension >::RegionType mitk::RandomParcellationGenerator<TPixel, VImageDimension>::ExtendedRegionNotNeighbor(typename itk::Image< TPixel, VImageDimension >::RegionType chosenRegion, typename itk::Image< TPixel, VImageDimension >::RegionType smallestRegion)
{
  itk::Size<3> sizeChosenRegion = chosenRegion.GetSize();
  itk::Index<3> startChosenRegion = chosenRegion.GetIndex();
  itk::Size<3> sizeSmallestRegion = smallestRegion.GetSize();
  itk::Index<3> startSmallestRegion = smallestRegion.GetIndex();

  //size and start of the new merged region
  itk::Size<3> size;
  itk::Index<3> start;

  //Calculate the size of the merged region
  for (int i = 0; i < 3; i++)
  {
    //Overlapping (start of ChosenRegion before start of SmallestRegion)
    if ((startChosenRegion[i] + sizeChosenRegion[i] > startSmallestRegion[i]) &&
      (startChosenRegion[i] <= startSmallestRegion[i]))
    {
      //Not integrated
      if (startSmallestRegion[i] + sizeSmallestRegion[i] > startChosenRegion[i] + sizeChosenRegion[i])
      {
        size[i] = abs(startSmallestRegion[i] - startChosenRegion[i]) + sizeSmallestRegion[i];
      }
      //Integrated
      else
      {
        size[i] = std::max(sizeChosenRegion[i], sizeSmallestRegion[i]);
      }
    }

    //Overlapping (start of SmallestRegion before start of ChosenRegion)
    if ((startSmallestRegion[i] + sizeSmallestRegion[i] > startChosenRegion[i]) &&
      (startChosenRegion[i] >= startSmallestRegion[i]))
    {
      //Not integrated
      if (startChosenRegion[i] + sizeChosenRegion[i] > startSmallestRegion[i] + sizeSmallestRegion[i])
      {
        size[i] = abs(startSmallestRegion[i] - startChosenRegion[i]) + sizeChosenRegion[i];
      }
      //Integrated
      else
      {
        size[i] = std::max(sizeChosenRegion[i], sizeSmallestRegion[i]);
      }
    }

    //No overlapping (right next to each other)
    if ((startChosenRegion[i] + sizeChosenRegion[i] == startSmallestRegion[i]) ||
      (startSmallestRegion[i] + sizeSmallestRegion[i] == startChosenRegion[i]))
    {
      size[i] = sizeChosenRegion[i] + sizeSmallestRegion[i];
    }

    //Isolated
    if ((startChosenRegion[i] + sizeChosenRegion[i] < startSmallestRegion[i]) ||
      (startSmallestRegion[i] + sizeSmallestRegion[i] < startChosenRegion[i]))
    {
      if(startChosenRegion[i] + sizeChosenRegion[i] < startSmallestRegion[i])
      {
        size[i] = abs(startChosenRegion[i] - startSmallestRegion[i]) +1;
      }
      if(startSmallestRegion[i] + sizeSmallestRegion[i] < startChosenRegion[i])
      {
        size[i] = abs(startChosenRegion[i] - startSmallestRegion[i]) + sizeChosenRegion[i];
      }
    }
  }

  //Calculate the start point of the new region, choose the smallest value respectively
  for (int i = 0; i < 3; i++)
  {
    start[i] = std::min(startChosenRegion[i], startSmallestRegion[i]);
  }

  chosenRegion.SetSize(size);
  chosenRegion.SetIndex(start);

  return chosenRegion;
}

//checks if a number is already an element of the vector
template < typename TPixel, unsigned int VImageDimension >
bool mitk::RandomParcellationGenerator<TPixel, VImageDimension>::IsUnique (int number, std::vector<int> vec)
{
  for (int i = 0; i < vec.size(); i++)
  {
    if (vec[i] == number)
    {
      return false;
    }
  }
  return true;
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::GetRandomSeedVoxels()
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  RegionVoxelCounter<int, 3> counter;
  counter.SetImage(m_Image);
  counter.SetRegion(m_Image->GetLargestPossibleRegion());

  int numberVoxels = counter.VoxelWithValue(1);

  //Create vector with unique numbers

  std::vector<int> randomvector;
  int randomnumber;

  //for-loop to get (m_NumberNodes) randomly chosen seed-points
  for (int j = 0; j < m_NumberNodes; j++)
  {
    //xrand() is the same as rand() but with an extended range
    randomnumber = xrand() % numberVoxels +1;

    //Bool-function (IsUnique)
    if (this->IsUnique(randomnumber, randomvector))
    {
      randomvector.push_back(randomnumber);
    }
    else
    {
      j--;
    }
  }

  //sorts the numbers in randomvector according to their size
  sort (randomvector.begin(), randomvector.end());

  //assign new values to the chosen voxels and build the regions

  itk::Size<3> originalSize;
  originalSize.Fill(1);
  itk::Index<3> originalStart;
  typename ImageType::IndexType currentIndex;

  int valueCounter(0);
  int regionNumber(0);
  typename ImageType::RegionType tempRegion;
  itk::ImageRegionIterator<ImageType> it_image(m_Image, m_Image->GetLargestPossibleRegion());


  for (it_image.GoToBegin(); !it_image.IsAtEnd(), regionNumber < m_NumberNodes; ++it_image)
  {
    if (it_image.Value() >= 1)
    {
      valueCounter++;
      if (valueCounter == randomvector[regionNumber])
      {
        it_image.Value() = regionNumber+3;

        //Build the Regions
        currentIndex = it_image.GetIndex();
        originalStart[0] = currentIndex[0];
        originalStart[1] = currentIndex[1];
        originalStart[2] = currentIndex[2];

        tempRegion.SetSize(originalSize);
        tempRegion.SetIndex(originalStart);
        std::pair<RegionType, int> tempPair (tempRegion, regionNumber+3);
        m_OddRegions.push_back(tempPair);
        regionNumber++;
      }
    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::ParcelGrowthOverFaces()
{
  typedef itk::Image< int, VImageDimension > IntegerImageType;
  typedef itk::NeighborhoodIterator< IntegerImageType > NeighborhoodIteratorType;
  typename NeighborhoodIteratorType::RadiusType radius;
  radius.Fill(1);

  std::pair<RegionType, int> chosenRegion;
  int thisRegion;
  typename IntegerImageType::IndexType indexChosenVoxel;
  std::vector<double> distance;
  std::vector<int> possibleNeighbor;
  int thisNeighbor;
  std::vector<double> centerOfMass;
  typename IntegerImageType::IndexType indexNewVoxel;
  std::vector<typename IntegerImageType::IndexType> indexNewVoxelVector;
  int numberAvailableVoxels(0);
  bool validVoxelNotYetFound;
  bool checkRegionsOfOddSize(true);
  std::vector<std::pair<RegionType,int> > * possibleRegions;
  int costValue;


  //As long as there are still non invalid regions (of even or odd size) add a voxel to them in each iteration step
  while(m_OddRegions.size() != 0 || m_EvenRegions.size() != 0)
  {
    //When we change from odd to even or vice versa
    if (m_OddRegions.size() == 0)
    {
      checkRegionsOfOddSize = false;
      possibleRegions = &m_EvenRegions;
    }

    if (m_EvenRegions.size() == 0)
    {
      checkRegionsOfOddSize = true;
      possibleRegions = &m_OddRegions;
    }

    //All our smallest regions are contained in possibleRegions, choose one randomly and check whether we can expand it

    thisRegion = xrand() % possibleRegions->size();
    chosenRegion = (*possibleRegions)[thisRegion];

    //Calculate Center of mass (COM)
    centerOfMass.clear();
    itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, chosenRegion.first);
    centerOfMass = this->GetCenterOfMass(it_region, chosenRegion.second, false);

    numberAvailableVoxels = 0;
    NeighborhoodIteratorType it_neighborhood(radius, m_Image, chosenRegion.first);

    //Check if there are any valid voxels which we can add to our chosen region

    for (it_neighborhood.GoToBegin(); !it_neighborhood.IsAtEnd(); ++it_neighborhood)
    {
      if (it_neighborhood.GetCenterPixel() == chosenRegion.second)
      {
        //Now the Neighborhood of a Parcel-Voxel is available
        //Check if there is a valid voxel

        for (int k = 0; k <27; k++)
        {
          if (it_neighborhood.GetPixel(k) == 1 && (k == 4 || k == 10 || k == 12 || k == 14 || k == 16 || k == 22) )
          {
            // Calculate the distance btw. the centerOfMass and the NewVoxel
            indexNewVoxel = it_neighborhood.GetIndex(k);
            indexNewVoxelVector.push_back(indexNewVoxel);

            distance.push_back(this->GetDistance(centerOfMass, indexNewVoxel));

            numberAvailableVoxels++;
          }
        }
      }
    }

    if (numberAvailableVoxels > 0)
    {
      validVoxelNotYetFound = true;
      while (validVoxelNotYetFound && distance.size() > 0)
      {
        possibleNeighbor.clear();
        //Find the possible neighbors with the smallest distance to the COM of the region

        for (int i = 0; i < distance.size(); i++)
        {
          if (distance[i] == this->SmallestValue(distance))
          {
            possibleNeighbor.push_back(i);
          }
        }

        //Choose a random voxel and check if it is valid
        //Get the index of this voxel
        thisNeighbor = xrand() % possibleNeighbor.size();
        indexChosenVoxel = indexNewVoxelVector[possibleNeighbor[thisNeighbor]];

        //Check if we now have to expand the region due to the possible new voxel
        std::pair<RegionType, int> chosenRegionChanged;
        chosenRegionChanged.first = this->ExtendedRegion(chosenRegion.first, indexChosenVoxel);
        chosenRegionChanged.second = chosenRegion.second;

        //Constraints

        CostFunctionBase<TPixel, VImageDimension> costCalculator;
        costCalculator.SetImage(m_Image);
        costCalculator.SetRegion(chosenRegionChanged);
        costValue = costCalculator.CalculateCost();


        if(costValue >= costCalculator.MaximalValue())//Constraints are fulfilled
        {
          validVoxelNotYetFound = false;
          chosenRegion = chosenRegionChanged;
        }
        else //Constraints are not fulfilled
        {
          validVoxelNotYetFound = true; //So we are still in the while loop
          distance.erase(distance.begin() + possibleNeighbor[thisNeighbor]);
          indexNewVoxelVector.erase(indexNewVoxelVector.begin() + possibleNeighbor[thisNeighbor]);

          if (distance.size() == 0)
          {
            if (checkRegionsOfOddSize == true)
            {
              m_InvalidRegions.push_back(chosenRegion);
              m_OddRegions.erase(m_OddRegions.begin() + thisRegion);
            }
            else
            {
              m_InvalidRegions.push_back(chosenRegion);
              m_EvenRegions.erase(m_EvenRegions.begin() + thisRegion);
            }
          }
        }
      }


      if (distance.size() > 0)
      {
        //Change the value of the new voxel
        itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_Image->GetLargestPossibleRegion());
        it_region.SetIndex(indexChosenVoxel);
        it_region.Value() = chosenRegion.second;

        //Change from even to odd or vice versa
        if (checkRegionsOfOddSize == true)
        {
          m_EvenRegions.push_back(chosenRegion);
          m_OddRegions.erase(m_OddRegions.begin() + thisRegion);
        }
        else
        {
          m_OddRegions.push_back(chosenRegion);
          m_EvenRegions.erase(m_EvenRegions.begin() + thisRegion);
        }

        distance.clear();
        indexNewVoxelVector.clear();
      }
    }
    else
    {
      //The region can't be extended, put it into the m_InvalidRegions vector
      m_InvalidRegions.push_back((*possibleRegions)[thisRegion]);
      possibleRegions->erase(possibleRegions->begin() + thisRegion);
    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::FillOverEdgeOrVertex()
{
  typedef itk::Image< int, VImageDimension > IntegerImageType;
  typedef itk::NeighborhoodIterator< IntegerImageType > NeighborhoodIteratorType;
  typename NeighborhoodIteratorType::RadiusType radius;
  radius.Fill(1);
  NeighborhoodIteratorType it_neighborhood(radius, m_Image, m_Image->GetLargestPossibleRegion());

  std::vector<int> valueOfRegions;
  std::vector<int> sizeOfPossibleRegions;
  std::vector<int> valueOfPossibleRegions;
  std::vector<int> indexInvalidRegions;
  std::vector<int> indexOfPossibleInvalidRegions;
  std::vector<double> distance;
  std::vector<int> possibleNeighbor;
  typename IntegerImageType::IndexType indexNewVoxel;
  std::vector<double> centerOfMass;
  int thisNeighbor;
  std::pair<RegionType, int> chosenRegion;
  typename IntegerImageType::IndexType indexChosenVoxel;

  RegionVoxelCounter<int, VImageDimension> wholeImageCounter;
  wholeImageCounter.SetImage(m_Image);
  wholeImageCounter.SetRegion(m_Image->GetLargestPossibleRegion());
  int voxelsWithValueOne = wholeImageCounter.VoxelWithValue(1);

  if (voxelsWithValueOne > 0)
  {
    std::stringstream message;
    message << "Edge " << voxelsWithValueOne << endl;
    MITK_DEBUG << message.str();
    for (it_neighborhood.GoToBegin(); !it_neighborhood.IsAtEnd(); ++it_neighborhood)
    {
      if (it_neighborhood.GetCenterPixel() == 1) //Found Voxel with Value 1
      {
        m_SizeOfRegions.clear();
        distance.clear();
        valueOfRegions.clear();
        possibleNeighbor.clear();
        indexInvalidRegions.clear();
        indexOfPossibleInvalidRegions.clear();
        indexNewVoxel = it_neighborhood.GetIndex();

        //Add this voxel to a region
        //first check if it shares an edge or vertex with (at least) one region
        for (int k = 0; k <27; k++)
        {
          if (it_neighborhood.GetPixel(k) > 2)
          {
            for (int i = 0; i < m_InvalidRegions.size(); i++)
            {
              if (it_neighborhood.GetPixel(k) == m_InvalidRegions[i].second)
              {
                valueOfRegions.push_back(m_InvalidRegions[i].second);
                itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_InvalidRegions[i].first);

                //Gets center of mass and the m_SizeOfRegions
                centerOfMass = this->GetCenterOfMass(it_region, m_InvalidRegions[i].second, true);
                distance.push_back(this->GetDistance(centerOfMass, indexNewVoxel));
                indexInvalidRegions.push_back(i);
                break;
              }
            }
          }
        }
        //Minimal distance
        if (distance.size() != 0)
        {
          for (int i = 0; i < distance.size(); i++)
          {
            if (distance[i] == this->SmallestValue(distance))
            {
              possibleNeighbor.push_back(i);
            }
          }
          distance.clear();

          //Check the Size of the Regions and get the smallest one!
          //First get the regions with the same distance to COM
          for (int i = 0; i < possibleNeighbor.size(); i++)
          {
            sizeOfPossibleRegions.push_back(m_SizeOfRegions[possibleNeighbor[i]]);
            valueOfPossibleRegions.push_back(valueOfRegions[possibleNeighbor[i]]);
            indexOfPossibleInvalidRegions.push_back(indexInvalidRegions[possibleNeighbor[i]]);
          }
          possibleNeighbor.clear();
          indexInvalidRegions.clear();

          for (int i = 0; i < sizeOfPossibleRegions.size(); i++)
          {
            if (sizeOfPossibleRegions[i] == this->SmallestValue(sizeOfPossibleRegions))
            {
              possibleNeighbor.push_back(i);
            }
          }
          sizeOfPossibleRegions.clear();

          thisNeighbor = xrand() % possibleNeighbor.size();

          it_neighborhood.SetCenterPixel(valueOfPossibleRegions[thisNeighbor]);
          valueOfPossibleRegions.clear();

          //region extansion if necessary
          //first we need to get the right region
          chosenRegion = m_InvalidRegions[indexOfPossibleInvalidRegions[thisNeighbor]];
          indexChosenVoxel = it_neighborhood.GetIndex();

          m_InvalidRegions[indexOfPossibleInvalidRegions[thisNeighbor]].first = this->ExtendedRegion(chosenRegion.first, indexChosenVoxel);

          indexOfPossibleInvalidRegions.clear();

        }
      }
    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::AllocateIsolatedVoxels()
{
  typedef itk::Image< int, VImageDimension > IntegerImageType;

  std::vector<int> valueOfRegions;
  std::vector<int> sizeOfPossibleRegions;
  std::vector<int> valueOfPossibleRegions;
  std::vector<int> indexInvalidRegions;
  std::vector<int> indexOfPossibleInvalidRegions;
  std::vector<double> distance;
  std::vector<int> possibleNeighbor;
  typename IntegerImageType::IndexType indexNewVoxel;
  std::vector<double> centerOfMass;
  int thisNeighbor;
  std::pair<RegionType, int> chosenRegion;
  typename IntegerImageType::IndexType indexChosenVoxel;

  RegionVoxelCounter<int, VImageDimension> wholeImageCounter;
  wholeImageCounter.SetImage(m_Image);
  wholeImageCounter.SetRegion(m_Image->GetLargestPossibleRegion());
  int voxelsWithValueOne = wholeImageCounter.VoxelWithValue(1);

  if (voxelsWithValueOne > 0)
  {
    std::stringstream message;
    message << "Isolated " << voxelsWithValueOne << endl;
    MITK_DEBUG << message.str();
    std::vector<std::vector<double> > comOfRegions;
    m_SizeOfRegions.clear();

    //Calculate all center of mass
    for (int i = 0; i < m_InvalidRegions.size(); i++)
    {
      valueOfRegions.push_back(m_InvalidRegions[i].second);
      itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_InvalidRegions[i].first);

      centerOfMass = this->GetCenterOfMass(it_region, m_InvalidRegions[i].second, true);
      comOfRegions.push_back(centerOfMass);
    }

    itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_Image->GetLargestPossibleRegion());

    for (it_region.GoToBegin(); !it_region.IsAtEnd(); ++it_region)
    {
      if (it_region.Value() == 1) //Found Voxel with Value 1
      {
        indexNewVoxel = it_region.GetIndex();

        //distance calculation
        for (int j = 0; j < m_InvalidRegions.size(); j++)
        {
          distance.push_back(this->GetDistance(comOfRegions[j], indexNewVoxel));
          indexInvalidRegions.push_back(j);

        }

        //Minimal distance
        if (distance.size() != 0)
        {
          for (int i = 0; i < distance.size(); i++)
          {
            if (distance[i] == this->SmallestValue(distance))
            {
              possibleNeighbor.push_back(i);
            }
          }
          distance.clear();

          //Check the Size of the Regions and get the smallest one!
          //First get the regions with the same distance to COM
          for (int i = 0; i < possibleNeighbor.size(); i++)
          {
            sizeOfPossibleRegions.push_back(m_SizeOfRegions[possibleNeighbor[i]]);
            valueOfPossibleRegions.push_back(valueOfRegions[possibleNeighbor[i]]);
            indexOfPossibleInvalidRegions.push_back(indexInvalidRegions[possibleNeighbor[i]]);
          }
          possibleNeighbor.clear();
          indexInvalidRegions.clear();

          for (int i = 0; i < sizeOfPossibleRegions.size(); i++)
          {
            if (sizeOfPossibleRegions[i] == this->SmallestValue(sizeOfPossibleRegions))
            {
              possibleNeighbor.push_back(i);
            }
          }
          sizeOfPossibleRegions.clear();

          thisNeighbor = xrand() % possibleNeighbor.size();

          it_region.Value() = valueOfPossibleRegions[thisNeighbor];
          valueOfPossibleRegions.clear();
          possibleNeighbor.clear();

          //region extansion if necessary
          //first we need to get the right region
          chosenRegion = m_InvalidRegions[indexOfPossibleInvalidRegions[thisNeighbor]];
          indexChosenVoxel = it_region.GetIndex();
          RegionType voxelRegion;
          voxelRegion.SetIndex(indexChosenVoxel);
          itk::Size<3> voxelSize;
          voxelSize.Fill(1);
          voxelRegion.SetSize(voxelSize);

          m_InvalidRegions[indexOfPossibleInvalidRegions[thisNeighbor]].first = this->ExtendedRegionNotNeighbor(chosenRegion.first, voxelRegion);

          indexOfPossibleInvalidRegions.clear();
        }
      }
    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
int mitk::RandomParcellationGenerator<TPixel, VImageDimension>::MergeParcels()
{
  typedef itk::Image< int, VImageDimension > IntegerImageType;
  typedef itk::NeighborhoodIterator< IntegerImageType > NeighborhoodIteratorType;
  typename NeighborhoodIteratorType::RadiusType radius;
  radius.Fill(1);

  //Calculate the m_SizeOfRegions
  this->GetSizeOfRegions();

  int sizeOfSmallestRegion = this->SmallestValue(m_SizeOfRegions);

  std::vector<int> smallRegions;
  std::vector<int> indexSmallRegions;
  int thisIndex;
  int thisSmallRegion;
  bool mergingStillPossible(true);
  std::vector<int> smallDistances;
  std::vector<double> centerOfMassPossibleRegion;
  std::vector<std::vector<double> > comOfRegions;
  std::vector<int> sizeSmallRegions;
  std::vector<int> smallestRegions;
  bool tooManyParcels(true);
  int currentNumberOfParcels = m_NumberNodes;
  int sizeNewParcel;
  std::vector<double> costFunctionValue;
  std::vector<double> smallestCostFunctionValues;
  std::vector<int> valueOfPossibleRegions;
  std::vector<double> distance;
  std::pair<RegionType, int> chosenRegion;
  std::vector<double> centerOfMass;
  bool smallRegionFound(false);
  bool hasNeighbors(false);

  while (sizeOfSmallestRegion < m_GivenSizeOfSmallestRegion && mergingStillPossible && tooManyParcels)
  {
    smallRegions.clear();
    sizeSmallRegions.clear();
    smallestRegions.clear();

    //Find all small Regions
    for (int i = 0; i < m_SizeOfRegions.size(); i++)
    {
      if (m_SizeOfRegions[i] < m_GivenSizeOfSmallestRegion)
      {
        smallRegions.push_back(i);
        sizeSmallRegions.push_back(m_SizeOfRegions[i]);
      }
    }

    //Find one of the smallest regions
    for (int i = 0; i < sizeSmallRegions.size(); i++)
    {
      if (sizeSmallRegions[i] == this->SmallestValue(sizeSmallRegions))
      {
        smallestRegions.push_back(smallRegions[i]);
      }
    }

    if (smallestRegions.size() > 0 && this->SmallestValue(sizeSmallRegions) < m_GivenSizeOfSmallestRegionBeginning)
    {
      thisSmallRegion = rand() % smallestRegions.size();

      //Choose a random small region
      //First check if it has direct neighbors
      NeighborhoodIteratorType it_neighborhood(radius, m_Image, m_InvalidRegions[smallestRegions[thisSmallRegion]].first);
      valueOfPossibleRegions.clear();
      smallRegionFound = false;
      hasNeighbors = false;

      for (it_neighborhood.GoToBegin(); !it_neighborhood.IsAtEnd(); ++it_neighborhood)
      {
        for (int k = 0; k <27; k++)
        {
          if ((k == 4 || k == 10 || k == 12 || k == 14 || k == 16 || k == 22) &&//over faces
            it_neighborhood.GetPixel(k) > 2                                     //found value which belongs to a region != the chosen region
            && it_neighborhood.GetPixel(k) != m_InvalidRegions[smallestRegions[thisSmallRegion]].second)
          {
            hasNeighbors = true;
            for (int i = 0; i < smallRegions.size(); i++)
            {
              if (it_neighborhood.GetPixel(k) == m_InvalidRegions[smallRegions[i]].second) //The value belongs to a small region
              {
                valueOfPossibleRegions.push_back(it_neighborhood.GetPixel(k));
                smallRegionFound = true;
                break;
              }
            }
          }
        }
      }

      if (hasNeighbors == true)
      {
        if (smallRegionFound == true)//the region has direct neighbors
        {
          //valueOfPossibleRegions may contain some values, that are the same. Erase the duplicates and sort the vector.
          std::sort(valueOfPossibleRegions.begin(), valueOfPossibleRegions.end());
          valueOfPossibleRegions.erase(std::unique(valueOfPossibleRegions.begin(), valueOfPossibleRegions.end()), valueOfPossibleRegions.end());

          indexSmallRegions.clear();

          //Get the values of the possible regions!
          for (int i = 0; i < valueOfPossibleRegions.size() ; i++)
          {
            for (int j = 0; j < m_InvalidRegions.size(); j++)
            {
              if (valueOfPossibleRegions[i] == m_InvalidRegions[j].second)
              {
                indexSmallRegions.push_back(j);
                break;
              }
            }
          }

          //take the region with the greatest value of the cost function
          //the cost function depends on the distance (btw the 2 COMs) and the size of the merged parcel

          //First get all the distances
          itk::ImageRegionIterator<IntegerImageType> it_regionSmallest(m_Image, m_InvalidRegions[smallestRegions[thisSmallRegion]].first);
          std::vector<double> centerOfMassCurrentRegion = this->GetCenterOfMass(it_regionSmallest, m_InvalidRegions[smallestRegions[thisSmallRegion]].second, false);

          distance.clear();

          for (int i = 0; i < indexSmallRegions.size(); i++)
          {
            itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_InvalidRegions[indexSmallRegions[i]].first);
            centerOfMassPossibleRegion = this->GetCenterOfMass(it_region, m_InvalidRegions[indexSmallRegions[i]].second, false);
            distance.push_back(this->GetDistanceVector(centerOfMassCurrentRegion, centerOfMassPossibleRegion));
          }
          smallDistances.clear();

          if (m_MergingWithSmallestParcel && m_JustMergeSmallParcels)
          {
            //If there are small Regions with equal distance btw the COMs choose a random one
            for (int i = 0; i < distance.size(); i++)
            {
              if (distance[i] == this->SmallestValue(distance))
              {
                smallDistances.push_back(indexSmallRegions[i]);
              }
            }
            thisIndex = smallDistances[rand() % smallDistances.size()];
          }

          //Calculate the cost function values

          if (m_MergingWithNumberParcels || (m_MergingWithSmallestParcel && !m_JustMergeSmallParcels))
          {
            costFunctionValue.clear();

            for (int i = 0; i < indexSmallRegions.size(); i++)
            {
              if (distance[i] == 0)
              {
                distance[i] = 0.01;
              }
              sizeNewParcel = (m_SizeOfRegions[smallestRegions[thisSmallRegion]] + m_SizeOfRegions[indexSmallRegions[i]]);

              costFunctionValue.push_back(-(1.0/distance[i]) - (1.0/sizeNewParcel)); //1, 2 or 3 at sizeNewParcel
            }
            smallestCostFunctionValues.clear();

            for (int i = 0; i < indexSmallRegions.size(); i++)
            {
              if (costFunctionValue[i] == this->SmallestValue(costFunctionValue))
              {
                smallestCostFunctionValues.push_back(indexSmallRegions[i]);
              }
            }
            thisIndex = smallestCostFunctionValues[rand() % smallestCostFunctionValues.size()];
          }

          //m_InvalidRegions[thisIndex].first is the region we want to merge with our current one

          //Colour the small parcel like the found one
          for (it_regionSmallest.GoToBegin(); !it_regionSmallest.IsAtEnd(); ++it_regionSmallest)
          {
            if (it_regionSmallest.Value() == m_InvalidRegions[smallestRegions[thisSmallRegion]].second)
            {
              it_regionSmallest.Value() = m_InvalidRegions[thisIndex].second;
            }
          }

          //Expand the region of the new parcel if necessary!

          chosenRegion = m_InvalidRegions[thisIndex];
          chosenRegion.first = this->ExtendedRegionNotNeighbor(chosenRegion.first, m_InvalidRegions[smallestRegions[thisSmallRegion]].first);
          m_InvalidRegions[thisIndex] = chosenRegion;

          //Erase the smallest Region from m_InvalidRegions
          m_InvalidRegions.erase(m_InvalidRegions.begin() + smallestRegions[thisSmallRegion]);
          currentNumberOfParcels--;

          if (currentNumberOfParcels <= m_DesiredNumberOfParcels)
          {
            tooManyParcels = false;
          }

          //m_SizeOfRegions changed, get the new one
          this->GetSizeOfRegions();
        }

        else //No merging possible, erase this small region from m_InvalidRegions and from m_SizeOfRegions
        {
          m_InvalidRegions.erase(m_InvalidRegions.begin() + smallestRegions[thisSmallRegion]);
          currentNumberOfParcels--;
          m_SizeOfFinishedRegions.push_back(m_SizeOfRegions[smallestRegions[thisSmallRegion]]);
          m_SizeOfRegions.erase(m_SizeOfRegions.begin() + smallestRegions[thisSmallRegion]);

          if (currentNumberOfParcels <= m_DesiredNumberOfParcels)
          {
            tooManyParcels = false;
          }
        }
      }
      else //There are no 6-connected neighborhood-regions. Try to merge this small region to a near region (according to the Center of Mass)
      {
        comOfRegions.clear();
        //Calculate the center of mass of all small regions
        for (int i = 0; i < smallRegions.size(); i++)
        {
          itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_InvalidRegions[smallRegions[i]].first);

          centerOfMass = this->GetCenterOfMass(it_region, m_InvalidRegions[smallRegions[i]].second, false);
          comOfRegions.push_back(centerOfMass);
        }

        //Calculate the distance between the center of mass of our small region and all other regions
        itk::ImageRegionIterator<IntegerImageType> it_regionSmallest(m_Image, m_InvalidRegions[smallestRegions[thisSmallRegion]].first);
        std::vector<double> centerOfMassCurrentRegion = this->GetCenterOfMass(it_regionSmallest, m_InvalidRegions[smallestRegions[thisSmallRegion]].second, false);

        distance.clear();

        for (int i = 0; i < smallRegions.size(); i++)
        {
          itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_InvalidRegions[smallRegions[i]].first);
          centerOfMassPossibleRegion = this->GetCenterOfMass(it_region, m_InvalidRegions[smallRegions[i]].second, false);
          distance.push_back(this->GetDistanceVector(centerOfMassCurrentRegion, centerOfMassPossibleRegion));
        }
        smallDistances.clear();
        std::vector<double> distanceWithoutZero = distance;
        for (int i = 0; i < distanceWithoutZero.size(); i++)
        {
          if (distanceWithoutZero[i] == 0)
          {
            distanceWithoutZero.erase(distanceWithoutZero.begin() + i); //Our smallestRegion is calculated too, erase the zero entry
            break;
          }
        }

        if (distanceWithoutZero.size() > 0)
        {
          //If there are small Regions with equal distance btw the COM's choose a random one
          for (int i = 0; i < distance.size(); i++)
          {
            if (distance[i] == this->SmallestValue(distanceWithoutZero))
            {
              smallDistances.push_back(smallRegions[i]);
            }
          }

          thisIndex = smallDistances[rand() % smallDistances.size()];

          //Colour the small parcel like the found one
          for (it_regionSmallest.GoToBegin(); !it_regionSmallest.IsAtEnd(); ++it_regionSmallest)
          {
            if (it_regionSmallest.Value() == m_InvalidRegions[smallestRegions[thisSmallRegion]].second)
            {
              it_regionSmallest.Value() = m_InvalidRegions[thisIndex].second;
            }
          }

          //Expand the region of the new parcel if necessary!
          chosenRegion = m_InvalidRegions[thisIndex];
          chosenRegion.first = this->ExtendedRegionNotNeighbor(chosenRegion.first , m_InvalidRegions[smallestRegions[thisSmallRegion]].first);
          m_InvalidRegions[thisIndex] = chosenRegion;

          //Erase the smallest Region from m_InvalidRegions
          m_InvalidRegions.erase(m_InvalidRegions.begin() + smallestRegions[thisSmallRegion]);
          currentNumberOfParcels--;

          if (currentNumberOfParcels <= m_DesiredNumberOfParcels)
          {
            tooManyParcels = false;
          }

          //m_SizeOfRegions changed, get the new one
          this->GetSizeOfRegions();
        }

        else//No merging possible, erase this small region from m_InvalidRegions and from m_SizeOfRegions
        {
          m_InvalidRegions.erase(m_InvalidRegions.begin() + smallestRegions[thisSmallRegion]);
          currentNumberOfParcels--;
          m_SizeOfFinishedRegions.push_back(m_SizeOfRegions[smallestRegions[thisSmallRegion]]);
          m_SizeOfRegions.erase(m_SizeOfRegions.begin() + smallestRegions[thisSmallRegion]);

          if (currentNumberOfParcels <= m_DesiredNumberOfParcels)
          {
            tooManyParcels = false;
          }
        }
      }
    }
    else //there are no parcels left to merge with
    {
      mergingStillPossible = false;
    }
  }
  return (m_SizeOfFinishedRegions.size() + m_SizeOfRegions.size());
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::GetSizeOfRegions()
{
  m_SizeOfRegions.clear();
  RegionVoxelCounter<int, VImageDimension> voxelCounter;
  voxelCounter.SetImage(m_Image);
  for (int i = 0; i < m_InvalidRegions.size(); i++)
  {
    voxelCounter.SetRegion(m_InvalidRegions[i].first);
    m_SizeOfRegions.push_back(voxelCounter.VoxelWithValue(m_InvalidRegions[i].second));
  }
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::RandomParcellationGenerator<TPixel, VImageDimension>::ShowSizeOfRegions()
{
  RegionVoxelCounter<int, VImageDimension> voxelCounter;
  voxelCounter.SetImage(m_Image);
  std::stringstream message;
  for (int i = 0; i < m_InvalidRegions.size(); i++)
  {
    voxelCounter.SetRegion(m_InvalidRegions[i].first);
    m_SizeOfRegions.push_back(voxelCounter.VoxelWithValue(m_InvalidRegions[i].second));
    message << voxelCounter.VoxelWithValue(m_InvalidRegions[i].second) << " , ";
  }
  MITK_DEBUG << message.str();
}
