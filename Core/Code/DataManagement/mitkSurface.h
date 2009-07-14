/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MITKSURFACEDATA_H_HEADER_INCLUDED
#define MITKSURFACEDATA_H_HEADER_INCLUDED

#include "mitkBaseData.h"
#include "itkImageRegion.h"

class vtkPolyData;

namespace mitk {

  //##Documentation
  //## @brief Class for storing surfaces (vtkPolyData)
  //## @ingroup Data
  class MITK_CORE_EXPORT Surface : public BaseData
  {
  protected:

  public:
    // not yet the best chioce of a region-type for surfaces, but it works for the time being
    typedef itk::ImageRegion< 5 >  RegionType;  

    mitkClassMacro(Surface, BaseData);

    itkNewMacro(Self);

    virtual void SetVtkPolyData(vtkPolyData* polydata, unsigned int t = 0);

    virtual vtkPolyData* GetVtkPolyData(unsigned int t = 0);

    virtual void UpdateOutputInformation();

    virtual void SetRequestedRegionToLargestPossibleRegion();

    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

    virtual bool VerifyRequestedRegion();

    virtual void SetRequestedRegion(itk::DataObject *data);

    virtual void SetRequestedRegion(Surface::RegionType *region);

    virtual void CopyInformation(const itk::DataObject *data);

    virtual bool IsEmpty(unsigned int t) const;

    unsigned int GetSizeOfPolyDataSeries() const;

    virtual void Update();

    virtual void Expand( unsigned int timeSteps = 1 );

    virtual void Graft( const DataObject* data );

    const RegionType& GetLargestPossibleRegion() const
    {
      m_LargestPossibleRegion.SetIndex(3, 0);
      m_LargestPossibleRegion.SetSize(3, GetTimeSlicedGeometry()->GetTimeSteps());
      return m_LargestPossibleRegion;
    }

    //##Documentation
    //## Get the region object that defines the size and starting index
    //## for the region of the image requested (i.e., the region of the
    //## image to be operated on by a filter).
    virtual const RegionType& GetRequestedRegion() const
    {
      return m_RequestedRegion;
    }

    void CalculateBoundingBox();

  virtual void ExecuteOperation(Operation *operation);

  protected:

    typedef std::vector< vtkPolyData* > VTKPolyDataSeries;

    Surface();

    virtual ~Surface();

    virtual void ClearData();

    virtual void InitializeEmpty();

    VTKPolyDataSeries m_PolyDataSeries;

    mutable RegionType m_LargestPossibleRegion;

    RegionType m_RequestedRegion;

    bool m_CalculateBoundingBox;
  };

} // namespace mitk

#endif /* MITKSURFACEDATA_H_HEADER_INCLUDED */
