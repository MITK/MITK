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
      typedef mitk::ScalarType ErrorType;

      
      //virtual const char* GetToolName() const;     ///< every tool has a name that can be used to identify it. 

      itkSetMacro(Position, PositionType);             ///< sets the position of the NavigationData object
      //itkSetVectorMacro(Position, PositionType, 3);    ///< sets the position of the NavigationData object
      itkGetConstMacro(Position, PositionType);        ///< returns position of the NavigationData object
      itkSetMacro(Orientation, OrientationType);       ///< sets the orientation of the NavigationData object
      //itkSetVectorMacro(Orientation, OrientationType, 4); ///< sets the orientation of the NavigationData object
      itkGetConstMacro(Orientation, OrientationType);  ///< returns the orientation of the NavigationData object
      virtual bool IsDataValid() const;                ///< returns true if the object contains valid data
      itkSetMacro(DataValid, bool);                    ///< sets the overall error estimation of the NavigationData object
      itkSetMacro(Error, ErrorType);     ///< sets the overall error estimation of the NavigationData object
      itkGetConstMacro(Error, ErrorType);     ///< return one value that corresponds to the overall tracking error.
      //itkGetMacro(TimeStamp, const mitk::TimeStamp*);   ///< returns the time when the position and orientation were received from the tracking device

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

      void SetTimeStamp( double Stamp );

      double GetTimeStamp();

    protected:
      NavigationData();
      virtual ~NavigationData();      
      //mitk::TrackingTool::Pointer m_Source; // \TODO is this needed`?
      //mitk::Matrix4D m_Pose; // \TODO is this needed?

      PositionType m_Position;
      OrientationType m_Orientation;     
      ErrorType m_Error;
      bool m_DataValid;
      typedef double TimeStamp;
      TimeStamp m_TimeStamp;

      //itk::FastMutexLock::Pointer m_MyMutex;
    };
} // namespace mitk

#endif /* MITKNAVIGATIONDATA_H_HEADER_INCLUDED_ */
