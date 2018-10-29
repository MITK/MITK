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

#ifndef MITKPHOTOACOUSTIC3dVOLUME_H
#define MITKPHOTOACOUSTIC3dVOLUME_H

#include "MitkPhotoacousticsLibExports.h"

//Includes for smart pointer usage
#include <mitkImage.h>
#include <itkLightObject.h>

namespace mitk
{
  namespace pa
  {
    /**
     * @brief The Volume class is designed to encapsulate volumetric information and to provide convenience methods
     * for data access and image conversions.
     */
    class MITKPHOTOACOUSTICSLIB_EXPORT Volume : public itk::LightObject
    {
    public:

      mitkClassMacroItkParent(Volume, itk::LightObject);

      /**
      *@brief returns smartpointer reference to a new instance of this objects.
      *  The given data array will be freed upon calling this constructor.
      *@param data
      *@param xDim
      *@param yDim
      *@param zDim
      *@param spacing
      *@return smartpointer reference to a new instance of this object
      */
      static Volume::Pointer New(double* data, unsigned int xDim, unsigned int yDim, unsigned int zDim, double spacing);

      static Volume::Pointer New(mitk::Image::Pointer image);

      /**
       * @brief GetData. Returns data at wanted position. For performance reasons, this method will not check,
       * if the specified position it within the array. Please use the GetXDim(), GetYDim() and GetZDim() methods
       * to check for yourself if necessary.
       *
       * @param x
       * @param y
       * @param z
       * @return the data contained within the data array held by this Volume at
       * positon x|y|z.
       */
      double GetData(unsigned int x, unsigned int y, unsigned int z);

      /**
      * Returns a const reference to the data encapsuled by this class.
      */
      double* GetData() const;

      /**
       * @brief SetData
       * @param data
       * @param x
       * @param y
       * @param z
       */
      void SetData(double data, unsigned int x, unsigned int y, unsigned int z);

      /**
       * @brief GetXDim
       * @return size of x dimension of this Volume
       */
      unsigned int GetXDim();

      /**
       * @brief GetYDim
       * @return size of y dimension of this Volume
       */
      unsigned int GetYDim();

      /**
       * @brief GetZDim
       * @return size of z dimension of this Volume
       */
      unsigned int GetZDim();

      /**
      *@brief returns the Volume instance as an mitk image
      */
      Image::Pointer AsMitkImage();

      /**
       * @brief DeepCopy
       * @return  a deep copy of this Volume. the old volume remains intact and memory is NOT shared
       * between the objects.
       */
      Volume::Pointer DeepCopy();

      /**
      *@brief convenience method to enable consistent access to the dat array
      *@return a 1d index from 3d pixel coordinates
      */
      long long GetIndex(unsigned int x, unsigned int y, unsigned int z);

      double GetSpacing();

      void SetSpacing(double spacing);

    protected:
      /**
       * @brief Initialize initializes this volume with the given pointer to the data array.
       * It is assumed, that the array is of dimension xDim|yDim|zDim.
       * The Photoacoustic3DVolume will handle memory management of the array and delete it on
       * constructor call.
       *
       * @param data a pointer to the data array
       * @param xDim x dimension of the data
       * @param yDim y dimension of the data
       * @param zDim z dimension of the data
       */
      Volume(double* data, unsigned int xDim, unsigned int yDim, unsigned int zDim, double spacing);
      Volume(mitk::Image::Pointer image);
      virtual ~Volume();

      const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;

      Image::Pointer m_InternalMitkImage;

      // this data is kept to enable fast access
      unsigned int m_XDim;
      unsigned int m_YDim;
      unsigned int m_ZDim;
      double* m_FastAccessDataPointer;
    };
  }
}

#endif // MITKPHOTOACOUSTIC3dVOLUME_H
