#include "BaseProcess.h"
#include "BaseData.h"

#define MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED

mitk::BaseProcess::BaseProcess() : m_Unregistering(false), m_RealReferenceCount(-1), m_CalculatingRealReferenceCount(false)
{

}

mitk::BaseProcess::~BaseProcess()
{

}

int mitk::BaseProcess::GetRealReferenceCount() const
{
    if(m_CalculatingRealReferenceCount==false) //this is only needed because a smart-pointer to m_Outputs (private!!) must be created by calling GetOutputs.
    {
        m_CalculatingRealReferenceCount = true;

        m_RealReferenceCount = -1;

        DataObjectPointerArray outputs = const_cast<mitk::BaseProcess*>(this)->GetOutputs();

        int realReferenceCount = GetReferenceCount();

        unsigned int idx;
        for (idx = 0; idx < outputs.size(); ++idx)
        {
            //references of outputs that are not referenced from someone else (reference additional to the reference from this BaseProcess object) are interpreted as non-existent 
            if((outputs[idx]) && (outputs[idx]->GetReferenceCount()==2)) //2 because the outputs array also holds a reference!
                --realReferenceCount;
        }
        m_RealReferenceCount = realReferenceCount;
        if(m_RealReferenceCount<0)
            m_RealReferenceCount=0;
    }
    else
        return -1;
    m_CalculatingRealReferenceCount = false; //do not move in if-part!!!
    return m_RealReferenceCount;
}

void mitk::BaseProcess::UnRegister() const
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
    if((m_Unregistering==false) && (m_CalculatingRealReferenceCount==false))
    {
        m_Unregistering=true;

        int realReferenceCount = GetRealReferenceCount();
        if(realReferenceCount<0)
            m_RealReferenceCount=0;

        if(realReferenceCount==0)
        {
            DataObjectPointerArray outputs = const_cast<mitk::BaseProcess*>(this)->GetOutputs();
            //disconnect all outputs from us
            unsigned int idx;
            for (idx = 0; idx < outputs.size(); ++idx)
            {
                const_cast<mitk::BaseProcess*>(this)->RemoveOutput(outputs[idx]);
            }
            //now the referenceCount should be one!
            int testReferenceCount=GetReferenceCount();
            if(testReferenceCount!=1)
                testReferenceCount=0;
        }
        m_Unregistering=false;
    }
    else
    {
        if(GetReferenceCount()==1)
        {
            //the calling UnRegister will do the last cleanup
            return;
        }
    }
#endif
    Superclass::UnRegister();
}

/**
* Set an output of this filter. This method specifically
* does not do a Register()/UnRegister() because of the 
* desire to break the reference counting loop.
*/
void mitk::BaseProcess::SetNthOutput(unsigned int idx, itk::DataObject *output)
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
    output = dynamic_cast<mitk::BaseData*>(output);

    // does this change anything?
    if ( idx < GetOutputs().size() && output == GetOutputs()[idx])
    {
        return;
    }

    if (output)
    {
        dynamic_cast<mitk::BaseData*>(output)->ConnectSource(this, idx);
    }
#endif
    Superclass::SetNthOutput(idx, output);
}

/**
* Adds an output to the first null position in the output list.
* Expands the list memory if necessary
*/
void mitk::BaseProcess::AddOutput(itk::DataObject *output)
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
    unsigned int idx;

    output = dynamic_cast<mitk::BaseData*>(output);

    if (output)
    {
        dynamic_cast<mitk::BaseData*>(output)->ConnectSource(this, idx);
    }
#endif
    Superclass::AddOutput(output);

}