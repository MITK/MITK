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
#include <mitkCommon.h>
#include <mitkVector.h>
#include <itkFastMutexLock.h>

//#include "mitkTimeStamp.h"

namespace mitk {

    /*Documentation
    * @brief Navigation Data
    *
    * This class represents the data object that is passed through the MITK-IGT navigation filter
    * pipeline. It encapsulates position and orientation of a tracked tool/sensor. Additionally, 
    * it contains a data structure that contains error/plausibility information
    * 
    * @ingroup Navigation
    */
  class NavigationData : public itk::DataObject
    {
    public:
      mitkClassMacro(NavigationData, itk::DataObject);
      itkNewMacro(Self);
      typedef mitk::Point3D PositionType;
      typedef mitk::Quaternion OrientationType;
      typedef itk::Matrix<mitk::ScalarType,6,6> CovarianceMatrixType;
      typedef double TimeStampType;

      
      //virtual const char* GetToolName() const;     ///< every tool has a name that can be used to identify it. 

      itkSetMacro(Position, PositionType);             ///< sets the position of the NavigationData object
      //itkSetVectorMacro(Position, PositionType, 3);    ///< sets the position of the NavigationData object
      itkGetConstMacro(Position, PositionType);        ///< returns position of the NavigationData object
      itkSetMacro(Orientation, OrientationType);       ///< sets the orientation of the NavigationData object
      //itkSetVectorMacro(Orientation, OrientationType, 4); ///< sets the orientation of the NavigationData object
      itkGetConstMacro(Orientation, OrientationType);  ///< returns the orientation of the NavigationData object
      virtual bool IsDataValid() const;                ///< returns true if the object contains valid data
      itkSetMacro(DataValid, bool);                    ///< sets the overall error estimation of the NavigationData object
      //itkGetMacro(TimeStamp, const mitk::TimeStamp*);   ///< returns the time when the position and orientation were received from the tracking device
      itkSetMacro(TimeStamp, TimeStampType);              ///< sets the timestamp of the NavigationData object
      itkGetMacro(TimeStamp, TimeStampType);              ///< gets the timestamp of the NavigationData object
      itkSetMacro(HasPosition, bool);                     ///< sets the HasPosition flag of the NavigationData object
      itkGetMacro(HasPosition, bool);                     ///< gets the HasPosition flag of the NavigationData object
      itkSetMacro(HasOrientation, bool);                  ///< sets the HasOrientation flag of the NavigationData object
      itkGetMacro(HasOrientation, bool);                  ///< gets the HasOrientation flag of the NavigationData object
      itkSetMacro(CovErrorMatrix, CovarianceMatrixType);  ///< sets the 6x6 Error Covariance Matrix of the NavigationData object
      itkGetMacro(CovErrorMatrix, CovarianceMatrixType);  ///< gets the 6x6 Error Covariance Matrix of the NavigationData object

      /** Graft the data and information from one NavigationData to another. This
      * is a convenience method to setup a second NavigationData object with all the meta
      * information of another NavigationData object.
      * Note that this method is different than just using two
      * SmartPointers to the same NavigationData object since separate DataObjects are
      * still maintained. 
      */
      virtual void Graft(const DataObject *data);
      
      /**Documentation
      * \brief copy content of data into itself
      *
      * copies all member values of data into this object (if data is a NavigationData object) 
      */
      virtual void CopyInformation(const DataObject* data);
      
      /**Documentation
      * \brief print object information
      */
      void PrintSelf(std::ostream& os, itk::Indent indent) const;

      /* Set the position part of m_CovErrorMatrix to I*error^2
      * This means that all position variables are assumed to be independent
      */
      void SetPositionAccuracy(mitk::ScalarType error);
      
      /* Set the orientation part of m_CovErrorMatrix to I*error^2
      * This means that all orientation variables are assumed to be independent
      */
      void SetOrientationAccuracy(mitk::ScalarType error);

    protected:
      NavigationData();
      virtual ~NavigationData();      
      //mitk::TrackingTool::Pointer m_Source; // \TODO is this needed`?
      //mitk::Matrix4D m_Pose; // \TODO is this needed?

      PositionType m_Position;
      OrientationType m_Orientation;     
           
      // A 6x6 covariance matrix parameterizing the Gaussian error
      // distribution of the measured position and orientation.
      // The hasPosition/hasOrientation fields define which entries
      // are valid.
      CovarianceMatrixType m_CovErrorMatrix;
      bool m_HasPosition;
      bool m_HasOrientation;
      
      bool m_DataValid;

      TimeStampType m_TimeStamp;

      //itk::FastMutexLock::Pointer m_MyMutex;
    };
} // namespace mitk

#endif /* MITKNAVIGATIONDATA_H_HEADER_INCLUDED_ */
