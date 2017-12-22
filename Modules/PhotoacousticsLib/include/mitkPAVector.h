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

#ifndef MITKSMARTVECTOR_H
#define MITKSMARTVECTOR_H

#include <mitkVector.h>
#include <random>

#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT Vector : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(Vector, itk::LightObject)
        itkFactorylessNewMacro(Self)

        /**
         * @brief GetNorm calculates the length of this vector.
         * @return the euclidean norm
         */
        double GetNorm();

      double GetElement(unsigned short index);
      void SetElement(unsigned short index, double value);

      /**
       * @brief Normalize normalizes this vector. After calling this GetNorm() will return 1.
       */
      void Normalize();
      void SetValue(Vector::Pointer value);

      /**
       * @brief RandomizeByPercentage alters this vector randomly by [-percentage, percentage] of the bendingFactor.
       *
       * @param percentage
       * @param bendingFactor
       */
      void RandomizeByPercentage(double percentage, double bendingFactor, std::mt19937* rng);

      /**
       * @brief Randomize randomizes this vector to be [lowerLimit, upperLimit] in each element
       *
       * @param xLowerLimit
       * @param xUpperLimit
       * @param yLowerLimit
       * @param yUpperLimit
       * @param zLowerLimit
       * @param zUpperLimit
       */
      void Randomize(double xLowerLimit, double xUpperLimit, double yLowerLimit, double yUpperLimit, double zLowerLimit, double zUpperLimit, std::mt19937* rng);

      /**
       * @brief Randomize randomizes this vector to be [0, limit] in each element
       *
       * @param xLimit
       * @param yLimit
       * @param zLimit
       */
      void Randomize(double xLimit, double yLimit, double zLimit, std::mt19937* rng);

      /**
       * @brief Randomize randomizes this vector to be [-1, 1] in each element
       */
      void Randomize(std::mt19937* rng);

      /**
       * @brief Rotate rotates this Vector around the x, y and z axis with the given angles in radians
       *
       * @param thetaChange rotation of the inclination angle in radians
       * @param phiChange rotation of the azimuthal angle in radians
       */
      void Rotate(double xAngle, double yAngle);

      /**
       * @brief Scale scales this Vector with the given factor
       *
       * @param factor the scaling factor
       *
       * If a negative number is provided, the direction of the vector will be inverted.
       */
      void Scale(double factor);

      /**
       * @brief Clone create a deep copy of this vector
       *
       * @return a new vector with the same values.
       */
      Vector::Pointer Clone();

    protected:
      Vector();
      ~Vector() override;

      void PrintSelf(std::ostream& os, itk::Indent indent) const override;

    private:
      mitk::Vector3D m_Vector;
    };

    /**
    * @brief Equal A function comparing two vectors for beeing equal
    *
    * @param rightHandSide A Vector to be compared
    * @param leftHandSide A Vector to be compared
    * @param eps tolarence for comparison. You can use mitk::eps in most cases.
    * @param verbose flag indicating if the user wants detailed console output or not.
    * @return true, if all subsequent comparisons are true, false otherwise
    */
    MITKPHOTOACOUSTICSLIB_EXPORT bool Equal(const Vector::Pointer leftHandSide, const Vector::Pointer rightHandSide, double eps, bool verbose);
  }
}
#endif // MITKSMARTVECTOR_H
