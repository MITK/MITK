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

#ifndef mitkCostFunctionBase_h
#define mitkCostFunctionBase_h

//Which one do we really need?
//#include <berryISelectionListener.h>
#include <itkImage.h>

#include <vector>
#include "mitkCommon.h"
#include "MitkConnectomicsExports.h"

//To use pair
#include <utility>

namespace mitk
{
  template <typename TPixel, unsigned int VImageDimension>
  class CostFunctionBase
  {
  public:
    typedef itk::ImageRegion<3> RegionType;
    typedef itk::Image< TPixel, VImageDimension > ImageType;
    //Set Functions
    void SetImage(itk::Image<TPixel, VImageDimension> *);
    void SetRegion(std::pair<RegionType, int>);
    //Main Function
    /** \brief Checks if the functions RegionIsFullEnough, SidesAreNotTooLong and COMLiesWithinParcel all return value 1*/
    int CalculateCost();
    /** \brief Calculates the maximal possible value of the cost function*/
    int MaximalValue();

  private:
    ImageType * m_Image;
    std::pair<RegionType, int> m_RegionPair;
    //Functions to calculate cost
    /** \brief Checks if the parcel fills out the region it lies within quit well
    * This is to get a convex structure.
    */
    int RegionIsFullEnough ();
    /** \brief Checks if the length of one direction is much longer than the length of the other two directions
    * So we don't get parcels which just grow in one direction and look like a line.
    */
    int SidesAreNotTooLong ();
    /** \brief Checks if the center of mass of a parcel lies within the parcel
    * This is to avoid U-shaped parcels.
    */
    int COMLiesWithinParcel();
    /** \brief Gives back the center of mass of a parcel as a vector*/
    std::vector<double> GetCenterOfMass();

    std::vector<int> m_weight;
  };
}

#include "mitkCostFunctionBase.cpp"

#endif /* mitkCostFunctionBase_h */