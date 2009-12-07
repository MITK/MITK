/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNAVIGATIONDATA_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATA_H_HEADER_INCLUDED_
#include <itkDataObject.h>
#include <MitkIGTExports.h>
#include <mitkVector.h>

namespace mitk {

    /**Documentation
    * \brief Navigation Data
    *
    * This class represents the data object that is passed through the MITK-IGT navigation filter
    * pipeline. It encapsulates position and orientation of a tracked tool/sensor. Additionally, 
    * it contains a data structure that contains error/plausibility information
    * 
    * \ingroup IGT
    */
  class MitkIGT_EXPORT NavigationData : public itk::DataObject
    {
    public:
      mitkClassMacro(NavigationData, itk::DataObject);
      itkNewMacro(Self);
      typedef mitk::Point3D PositionType;         ///< Type that holds the position part of the tracking data
      typedef mitk::Quaternion OrientationType;   ///< Type that holds the orientation part of the tracking data
      typedef itk::Matrix<mitk::ScalarType,6,6> CovarianceMatrixType; ///< type that holds the error characterization of the position and orientation measurements
      typedef double TimeStampType;   ///< type that holds the time at which the data was recorded


      itkSetMacro(Position, PositionType);             ///< sets the position of the NavigationData object
      itkGetConstMacro(Position, PositionType);        ///< returns position of the NavigationData object
      itkSetMacro(Orientation, OrientationType);       ///< sets the orientation of the NavigationData object
      itkGetConstMacro(Orientation, OrientationType);  ///< returns the orientation of the NavigationData object
      virtual bool IsDataValid() const;                ///< returns true if the object contains valid data
      itkSetMacro(DataValid, bool);                    ///< sets the overall error estimation of the NavigationData object
      itkSetMacro(TimeStamp, TimeStampType);              ///< sets the timestamp of the NavigationData object
      itkGetConstMacro(TimeStamp, TimeStampType);         ///< gets the timestamp of the NavigationData object
      itkSetMacro(HasPosition, bool);                     ///< sets the HasPosition flag of the NavigationData object
      itkGetConstMacro(HasPosition, bool);                ///< gets the HasPosition flag of the NavigationData object
      itkSetMacro(HasOrientation, bool);                  ///< sets the HasOrientation flag of the NavigationData object
      itkGetConstMacro(HasOrientation, bool);             ///< gets the HasOrientation flag of the NavigationData object
      itkSetMacro(CovErrorMatrix, CovarianceMatrixType);  ///< sets the 6x6 Error Covariance Matrix of the NavigationData object
      itkGetConstMacro(CovErrorMatrix, CovarianceMatrixType);  ///< gets the 6x6 Error Covariance Matrix of the NavigationData object
      itkSetStringMacro(Name);                            ///< set the name of the NavigationData object
      itkGetStringMacro(Name);                            ///< returns the name of the NavigationData object

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
      * \brief print object information
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

    protected:
      NavigationData();
      virtual ~NavigationData();

      PositionType m_Position;        ///< holds the position part of the tracking data
      OrientationType m_Orientation;  ///< holds the orientation part of the tracking data

      /**
       * \brief A 6x6 covariance matrix parameterizing the Gaussian error
       * distribution of the measured position and orientation.
       *
       * The hasPosition/hasOrientation fields define which entries
       * are valid.
       */
      CovarianceMatrixType m_CovErrorMatrix;  ///< holds the error characterization of the position and orientation
      bool m_HasPosition;     ///< defines if position part of m_CovErrorMatrix is valid
      bool m_HasOrientation;  ///< defines if orientation part of m_CovErrorMatrix is valid
      bool m_DataValid;       ///< defines if the object contains valid values
      TimeStampType m_TimeStamp;  ///< contains the time at which the tracking data was recorded
      std::string m_Name;       ///< name of the navigation data
    };
} // namespace mitk
#endif /* MITKNAVIGATIONDATA_H_HEADER_INCLUDED_ */
