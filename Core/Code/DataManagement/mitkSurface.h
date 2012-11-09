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
    mitkCloneMacro(Surface);

    virtual void SetVtkPolyData(vtkPolyData* polydata, unsigned int t = 0);

    virtual vtkPolyData* GetVtkPolyData(unsigned int t = 0);

    virtual void UpdateOutputInformation();

    virtual void SetRequestedRegionToLargestPossibleRegion();

    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

    virtual bool VerifyRequestedRegion();

    virtual void SetRequestedRegion(itk::DataObject *data);

    virtual void SetRequestedRegion(Surface::RegionType *region);

    virtual void CopyInformation(const itk::DataObject *data);

    virtual bool IsEmptyTimeStep(unsigned int t) const;

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

    virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const;

    virtual void ExecuteOperation(Operation *operation);

  protected:

    typedef std::vector< vtkPolyData* > VTKPolyDataSeries;

    Surface();
    Surface(const Surface& other);

    virtual ~Surface();

    virtual void ClearData();

    virtual void InitializeEmpty();

    //member variables
    VTKPolyDataSeries m_PolyDataSeries;           /// variable holds the poly datas of the surface
    mutable RegionType m_LargestPossibleRegion;   /// variable holds the largest possible region the surface is contained in
    RegionType m_RequestedRegion;
    bool m_CalculateBoundingBox;                 /// flag to calculate the bounding box
  };

} // namespace mitk

#endif /* MITKSURFACEDATA_H_HEADER_INCLUDED */
