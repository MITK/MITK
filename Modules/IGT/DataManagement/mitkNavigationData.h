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


#ifndef MITKNAVIGATIONDATA_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATA_H_HEADER_INCLUDED_
#include <itkDataObject.h>
#include <MitkIGTExports.h>
#include <mitkCommon.h>
#include <mitkNumericTypes.h>

namespace mitk {

    /**Documentation
    * \brief Navigation Data
    *
    * This class represents the data object that is passed through the MITK-IGT navigation filter
    * pipeline. It encapsulates position and orientation of a tracked tool/sensor. Additionally,
    * it contains a data structure that contains error/plausibility information
    *
    * It provides methods to work with the affine transformation represented by its orientation and position.
    * Additionally, it provides a constructor to construct a NavigationData object from an AffineTransform3D and
    * a getter to create an AffineTransform3D from a NavigationData object.
    *
    * \ingroup IGT
    */
  class MitkIGT_EXPORT NavigationData : public itk::DataObject
    {
    public:
      mitkClassMacro(NavigationData, itk::DataObject);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);
      mitkNewMacro2Param(Self, mitk::AffineTransform3D::Pointer, const bool);
      mitkNewMacro1Param(Self, mitk::AffineTransform3D::Pointer);

      /**
      * \brief Type that holds the position part of the tracking data
      */
      typedef mitk::Point3D PositionType;
      /**
      * \brief Type that holds the orientation part of the tracking data
      */
      typedef mitk::Quaternion OrientationType;
      /**
      * \brief type that holds the error characterization of the position and orientation measurements
      */
      typedef itk::Matrix<mitk::ScalarType,6,6> CovarianceMatrixType;
      /**
      * \brief type that holds the time at which the data was recorded
      */
      typedef double TimeStampType;

      /**
      * \brief sets the position of the NavigationData object
      */
      itkSetMacro(Position, PositionType);
      /**
      * \brief returns position of the NavigationData object
      */
      itkGetConstMacro(Position, PositionType);
      /**
      * \brief sets the orientation of the NavigationData object
      */
      itkSetMacro(Orientation, OrientationType);
      /**
      * \brief returns the orientation of the NavigationData object
      */
      itkGetConstMacro(Orientation, OrientationType);
      /**
      * \brief returns true if the object contains valid data
      */
      virtual bool IsDataValid() const;
      /**
      * \brief sets the dataValid flag of the NavigationData object indicating if the object contains valid data
      */
      itkSetMacro(DataValid, bool);
      /**
      * \brief sets the IGT timestamp of the NavigationData object
      */
      itkSetMacro(IGTTimeStamp, TimeStampType);
      /**
      * \brief gets the IGT timestamp of the NavigationData object
      */
      itkGetConstMacro(IGTTimeStamp, TimeStampType);
      /**
      * \brief sets the HasPosition flag of the NavigationData object
      */
      itkSetMacro(HasPosition, bool);
      /**
      * \brief gets the HasPosition flag of the NavigationData object
      */
      itkGetConstMacro(HasPosition, bool);
      /**
      * \brief sets the HasOrientation flag of the NavigationData object
      */
      itkSetMacro(HasOrientation, bool);
      /**
      * \brief gets the HasOrientation flag of the NavigationData object
      */
      itkGetConstMacro(HasOrientation, bool);
      /**
      * \brief sets the 6x6 Error Covariance Matrix of the NavigationData object
      */
      itkSetMacro(CovErrorMatrix, CovarianceMatrixType);
      /**
      * \brief gets the 6x6 Error Covariance Matrix of the NavigationData object
      */
      itkGetConstMacro(CovErrorMatrix, CovarianceMatrixType);
      /**
      * \brief set the name of the NavigationData object
      */
      itkSetStringMacro(Name);
      /**
      * \brief returns the name of the NavigationData object
      */
      itkGetStringMacro(Name);

      /**
      * \brief Graft the data and information from one NavigationData to another.
      *
      * Copies the content of data into this object.
      * This is a convenience method to setup a second NavigationData object with all the meta
      * information of another NavigationData object.
      * Note that this method is different than just using two
      * SmartPointers to the same NavigationData object since separate DataObjects are
      * still maintained.
      */
      virtual void Graft(const DataObject *data);

      /**
      * \brief copy meta data of a NavigationData object
      *
      * copies all meta data from NavigationData data to this object
      */
      virtual void CopyInformation(const DataObject* data);

      /**
      * \brief Prints the object information to the given stream os.
      * \param os The stream which is used to print the output.
      * \param indent Defines the indentation of the output.
      */
      void PrintSelf(std::ostream& os, itk::Indent indent) const;

      /**
      * Set the position part of m_CovErrorMatrix to I*error^2
      * This means that all position variables are assumed to be independent
      */
      void SetPositionAccuracy(mitk::ScalarType error);

      /**
      * Set the orientation part of m_CovErrorMatrix to I*error^2
      * This means that all orientation variables are assumed to be independent
      */
      void SetOrientationAccuracy(mitk::ScalarType error);

      /**
       * \brief Calculate AffineTransform3D from the transformation held by this NavigationData.
       * TODO: should throw an error if transformation is invalid.
       */
      mitk::AffineTransform3D::Pointer GetAffineTransform3D() const;

      /**
       * \brief Calculate the RotationMatrix of this transformation.
       */
      mitk::Matrix3D GetRotationMatrix() const;

      /**
       * \brief Transform by an affine transformation
       *
       * This method applies the affine transform given by self to a
       * given point, returning the transformed point.
       */
      mitk::Point3D TransformPoint(const mitk::Point3D point) const;

      /**
       * Get inverse of the Transformation represented by this NavigationData.
       * @throws mitk::Exception in case the transformation is invalid (only case: quaternion is zero)
       */
      mitk::NavigationData::Pointer GetInverse() const;

      /** Compose with another NavigationData
       *
       * This method composes self with another NavigationData of the
       * same dimension, modifying self to be the composition of self
       * and other.  If the argument pre is true, then other is
       * precomposed with self; that is, the resulting transformation
       * consists of first applying other to the source, followed by
       * self.  If pre is false or omitted, then other is post-composed
       * with self; that is the resulting transformation consists of
       * first applying self to the source, followed by other. */
      void Compose(const mitk::NavigationData::Pointer n, const bool pre = false);

    protected:
      mitkCloneMacro(Self);

      NavigationData();

      /*
       * Copy constructor internally used.
       */
      NavigationData(const mitk::NavigationData& toCopy);

      /**
       * Creates a NavigationData object from an affineTransform3D.
       * Caution: NavigationData doesn't support spacing, only translation and rotation. If the affine
       * transform includes spacing it cannot be converted to a NavigationData and an exception is thrown.
       * @param checkForRotationMatrix  if this is true, the rotation matrix coming from the affineTransform is checked
       *  for being a rotation matrix. If it isn't, an exception is thrown. Disable this check by
       *  setting checkForRotationMatrix to false.
       *
       *  @throws mitkException if checkForRotationMatrix is true and a non rotation matrix was introduced by
       *    AffineTransform.
       */
      NavigationData(mitk::AffineTransform3D::Pointer affineTransform3D, const bool checkForRotationMatrix = true);

      virtual ~NavigationData();

      /**
      * \brief holds the position part of the tracking data
      */
      PositionType m_Position;
      /**
      * \brief holds the orientation part of the tracking data
      */
      OrientationType m_Orientation;

      /**
       * \brief A 6x6 covariance matrix parameterizing the Gaussian error
       * distribution of the measured position and orientation.
       *
       * The hasPosition/hasOrientation fields define which entries
       * are valid.
       */
      CovarianceMatrixType m_CovErrorMatrix;  ///< holds the error characterization of the position and orientation
      /**
      * \brief defines if position part of m_CovErrorMatrix is valid
      */
      bool m_HasPosition;
      /**
      * \brief defines if orientation part of m_CovErrorMatrix is valid
      */
      bool m_HasOrientation;
      /**
      * \brief defines if the object contains valid values
      */
      bool m_DataValid;
      /**
      * \brief contains the time at which the tracking data was recorded
      */
      TimeStampType m_IGTTimeStamp;
      /**
      * \brief name of the navigation data
      */
      std::string m_Name;

    private:

      void ResetCovarianceValidity();

      // pre = false
      static mitk::NavigationData::Pointer getComposition(const mitk::NavigationData::Pointer nd1, const mitk::NavigationData::Pointer nd2);

    };

    /**
      * @brief Equal A function comparing two navigation data objects for beeing equal in meta- and imagedata
      *
      * @ingroup MITKTestingAPI
      *
      * Following aspects are tested for equality:
      *  - position
      *  - orientation
      *  - other members and flags of the class
      *
      * @param rightHandSide An NavigationData to be compared
      * @param leftHandSide An NavigationData to be compared
      * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
      * @param verbose Flag indicating if the user wants detailed console output or not.
      * @return true, if all subsequent comparisons are true, false otherwise
      */
      MitkIGT_EXPORT bool Equal( const mitk::NavigationData& leftHandSide, const mitk::NavigationData& rightHandSide, ScalarType eps = mitk::eps, bool verbose = false );

} // namespace mitk
#endif /* MITKNAVIGATIONDATA_H_HEADER_INCLUDED_ */
