#ifndef BASEDATA_H_HEADER_INCLUDED_C1EBB6FA
#define BASEDATA_H_HEADER_INCLUDED_C1EBB6FA

#include <itkDataObject.h>

#include "mitkCommon.h"
#include "Geometry3D.h"
#include "Geometry2D.h"
#include "PropertyList.h"

namespace mitk {

class BaseProcess;

//##ModelId=3D6A0D7D00EC
//##Documentation
//## @brief Base of all data objects
//## @ingroup Data
//## Base of all data objects, e.g., images, contours, surfaces etc. Inherits
//## from itk::DataObject and thus can be included in a pipeline.
class BaseData : public itk::DataObject
{
public:
    //##ModelId=3E10262200CE
    typedef Geometry3D::Pointer Geometry3DPointer;

    mitkClassMacro(BaseData,itk::DataObject)

    //itkNewMacro(Self);  

    //##ModelId=3DCBE2BA0139
    //##Documentation
    //## @brief Return the Geometry3D of the data. 
	//## 
	//## The method does not simply return the value of the m_Geometry3D member. 
	//## Before doing this, it
    //## makes sure that the Geometry3D is up-to-date before returning it (by
    //## setting the update extent appropriately and calling
    //## UpdateOutputInformation).
    //## 
    //## @warning GetGeometry not yet completely implemented. Appropriate
    //## setting of the update extent is missing.
        mitk::Geometry3D::ConstPointer GetGeometry() const;
    //##ModelId=3E3C4ACB0046
    //##Documentation
    //## @brief Return the Geometry2D of the slice (@a s, @a t). 
	//## 
	//## The method does not
    //## simply call GetGeometry()->GetGeometry2D(). Before doing this, it
    //## makes sure that the Geometry2D is up-to-date before returning it (by
    //## setting the update extent appropriately and calling
    //## UpdateOutputInformation).
    //## 
    //## @warning GetGeometry2D not yet completely implemented. Appropriate
    //## setting of the update extent is missing.
    virtual mitk::Geometry2D::ConstPointer GetGeometry2D(int s, int t) const;

    //##ModelId=3E8600DB0188
    //##Documentation
    //## @brief Helps to deal with the weak-pointer-problem.
    virtual void UnRegister() const;

    //##ModelId=3E8600DB02DC
    //##Documentation
    //## @brief for internal use only. Helps to deal with the
    //## weak-pointer-problem.
    virtual int GetExternalReferenceCount() const;

   //##ModelId=3EDD06DC017A
   //##Documentation 
   //## @brief Update the information for this BaseData so that it can be used
   //## as an output of a BaseProcess. 
   //## 
   //## This method is used in the pipeline mechanism to propagate information and 
   //## initialize the meta data associated with a BaseData. Any implementation 
   //## of this method in a derived class is assumed to call its source's
   //## BaseProcess::UpdateOutputInformation() which determines modified
   //## times, LargestPossibleRegions, and any extra meta data like spacing,
   //## origin, etc. 
   void UpdateOutputInformation()=0;

   //##ModelId=3EDD06DC035E
   //##Documentation 
   //## @brief Set the RequestedRegion to the LargestPossibleRegion.
   //## 
   //## This forces a filter to produce all of the output in one execution
   //## (i.e. not streaming) on the next call to Update(). 
  void SetRequestedRegionToLargestPossibleRegion()=0;

   //##ModelId=3EDD06DD017A
   //##Documentation 
   //## @brief Determine whether the RequestedRegion is outside of the BufferedRegion.
   //## 
   //## This method returns true if the RequestedRegion
   //## is outside the BufferedRegion (true if at least one pixel is
   //## outside). This is used by the pipeline mechanism to determine
   //## whether a filter needs to re-execute in order to satisfy the
   //## current request.  If the current RequestedRegion is already
   //## inside the BufferedRegion from the previous execution (and the
   //## current filter is up to date), then a given filter does not need
   //## to re-execute 
  bool RequestedRegionIsOutsideOfTheBufferedRegion()=0;

   //##ModelId=3EDD09370191
   //##Documentation
   //## @brief Verify that the RequestedRegion is within the LargestPossibleRegion.
   //## 
   //## If the RequestedRegion is not within the LargestPossibleRegion,
   //## then the filter cannot possibly satisfy the request. This method
   //## returns true if the request can be satisfied (even if it will be
   //## necessary to process the entire LargestPossibleRegion) and
   //## returns false otherwise.  This method is used by
   //## PropagateRequestedRegion().  PropagateRequestedRegion() throws a
   //## InvalidRequestedRegionError exception if the requested region is
   //## not within the LargestPossibleRegion. 
  virtual bool VerifyRequestedRegion() = 0;

   //##ModelId=3EDD06DE0274
   //##Documentation 
   //## @brief Copy information from the specified data set.
   //## 
   //## This method is part of the pipeline execution model. By default, a 
   //## BaseProcess will copy meta-data from the first input to all of its
   //## outputs. See ProcessObject::GenerateOutputInformation().  Each
   //## subclass of DataObject is responsible for being able to copy
   //## whatever meta-data it needs from from another DataObject.
   //## The default implementation of this method is empty. If a subclass
   //## overrides this method, it should always call its superclass'
   //## version. 
  void CopyInformation(const itk::DataObject*) {};
  
   //##ModelId=3EDD06DF017A
   //##Documentation
   //## @brief Set the requested region from this data object to match the requested
   //## region of the data object passed in as a parameter.
   //## 
   //## This method is implemented in the concrete subclasses of BaseData. 
  void SetRequestedRegion(itk::DataObject *data)=0;
protected:
    //##ModelId=3E3FE04202B9
    BaseData();
    //##ModelId=3E3FE042031D
    ~BaseData();
    //##ModelId=3E15551A03CE
    Geometry3DPointer m_Geometry3D;

    //##ModelId=3E8600D9006D
    mutable itk::SmartPointer<mitk::BaseProcess> m_SmartSourcePointer;
    //##ModelId=3E8600D9021B
    mutable unsigned int m_SourceOutputIndexDuplicate;
    //##ModelId=3E8600DC0053
    //##Documentation
    //## @brief for internal use only. Helps to deal with the
    //## weak-pointer-problem.
    virtual void ConnectSource(itk::ProcessObject *arg, unsigned int idx) const;
private:
    //##ModelId=3E8600D90384
    //##Documentation
    //## @brief Helps to deal with the weak-pointer-problem.
    mutable bool m_Unregistering;
    //##ModelId=3E8600DA0118
    //##Documentation
    //## @brief Helps to deal with the weak-pointer-problem.
    mutable bool m_CalculatingExternalReferenceCount;
    //##ModelId=3E8600DA02B3
    //##Documentation
    //## @brief Helps to deal with the weak-pointer-problem.
    mutable int m_ExternalReferenceCount;

    //##Documentation
    //## @brief Helps to deal with the weak-pointer-problem.
    friend class mitk::BaseProcess;
};

} // namespace mitk


#endif /* BASEDATA_H_HEADER_INCLUDED_C1EBB6FA */
