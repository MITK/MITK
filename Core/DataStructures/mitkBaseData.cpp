#include "BaseData.h"
#include "BaseProcess.h"

#define MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED

//##ModelId=3E3FE04202B9
mitk::BaseData::BaseData() : m_SmartSourcePointer(NULL), m_SourceOutputIndexDuplicate(0), m_Unregistering(false), m_ExternalReferenceCount(-1), m_CalculatingExternalReferenceCount(false)
{

}

//##ModelId=3E3FE042031D
mitk::BaseData::~BaseData() 
{
    m_SmartSourcePointer = NULL;
}

//##ModelId=3DCBE2BA0139
mitk::Geometry3D::Pointer mitk::BaseData::GetGeometry() const
{
    const_cast<BaseData*>(this)->SetRequestedRegionToLargestPossibleRegion();

    const_cast<BaseData*>(this)->UpdateOutputInformation();

    return m_Geometry3D.GetPointer();
}

//##ModelId=3E3C4ACB0046
mitk::Geometry2D::ConstPointer mitk::BaseData::GetGeometry2D(int s, int t) const
{
    const_cast<BaseData*>(this)->SetRequestedRegionToLargestPossibleRegion();

    const_cast<BaseData*>(this)->UpdateOutputInformation();

    return m_Geometry3D->GetGeometry2D(s,t);
}

//##ModelId=3E8600DB02DC
int mitk::BaseData::GetExternalReferenceCount() const
{
    if(m_CalculatingExternalReferenceCount==false) //this is only needed because a smart-pointer to m_Outputs (private!!) must be created by calling GetOutputs.
    {
        m_CalculatingExternalReferenceCount = true;

        m_ExternalReferenceCount = -1;

        int realReferenceCount = GetReferenceCount();

        if(GetSource()==NULL) 
        {
            m_ExternalReferenceCount = realReferenceCount;
            m_CalculatingExternalReferenceCount = false;
            return m_ExternalReferenceCount;
        }

        mitk::BaseProcess::DataObjectPointerArray outputs = m_SmartSourcePointer->GetOutputs();

        unsigned int idx;
        for (idx = 0; idx < outputs.size(); ++idx)
        {
            //references of outputs that are not referenced from someone else (reference additional to the reference from this BaseProcess object) are interpreted as non-existent 
            if(outputs[idx]==this)
                --realReferenceCount;
        }
        m_ExternalReferenceCount = realReferenceCount;
        if(m_ExternalReferenceCount<0)
            m_ExternalReferenceCount=0;
        m_CalculatingExternalReferenceCount = false;
    }
    else
        return -1;
    return m_ExternalReferenceCount;
}

//##ModelId=3E8600DB0188
void mitk::BaseData::UnRegister() const
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
    if(GetReferenceCount()>1)
    {
        Superclass::UnRegister();
        if((m_Unregistering==false) && (m_SmartSourcePointer.IsNotNull()))
        {
            m_Unregistering=true;
            if(m_SmartSourcePointer->GetExternalReferenceCount()==0)
                m_SmartSourcePointer=NULL; // now the reference count is zero and this object has been destroyed; thus nothing may be done after this line!!
            else
                m_Unregistering=false;
        }
    }
    else
#endif
        Superclass::UnRegister(); // now the reference count is zero and this object has been destroyed; thus nothing may be done after this line!!
}

//##ModelId=3E8600DC0053
void mitk::BaseData::ConnectSource(itk::ProcessObject *arg, unsigned int idx) const
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
    itkDebugMacro( "connecting source  " << arg
        << ", source output index " << idx);

    if ( GetSource() != arg || m_SourceOutputIndexDuplicate != idx)
    {
        m_SmartSourcePointer = dynamic_cast<mitk::BaseProcess*>(arg);
        m_SourceOutputIndexDuplicate = idx;
        Modified();
    }
#endif
}

