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

#include "mitkBaseProcess.h"
#include "mitkBaseData.h"


#define MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED

//##ModelId=3E8600DD0036
mitk::BaseProcess::BaseProcess() : m_Unregistering(false), m_CalculatingExternalReferenceCount(false), m_ExternalReferenceCount(-1)
{

}

//##ModelId=3E8600DD004A
mitk::BaseProcess::~BaseProcess()
{

}

//##ModelId=3E8600DD000E
int mitk::BaseProcess::GetExternalReferenceCount() const
{
  if(m_CalculatingExternalReferenceCount==false) //this is only needed because a smart-pointer to m_Outputs (private!!) must be created by calling GetOutputs.
  {
    m_CalculatingExternalReferenceCount = true;

    m_ExternalReferenceCount = -1;

    DataObjectPointerArray& outputs = const_cast<mitk::BaseProcess*>(this)->GetOutputs();

    int realReferenceCount = GetReferenceCount();

    unsigned int idx;
    for (idx = 0; idx < outputs.size(); ++idx)
    {
      //references of outputs that are not referenced from someone else (reference additional to the reference from this BaseProcess object) are interpreted as non-existent 
      if((outputs[idx]) && (outputs[idx]->GetReferenceCount()==1))
        --realReferenceCount;
    }
    m_ExternalReferenceCount = realReferenceCount;
    if(m_ExternalReferenceCount<0)
      m_ExternalReferenceCount=0;
  }
  else
    return -1;
  m_CalculatingExternalReferenceCount = false; //do not move in if-part!!!
  return m_ExternalReferenceCount;
}

//##ModelId=3E8600DC03E2
void mitk::BaseProcess::UnRegister() const
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
  if((m_Unregistering==false) && (m_CalculatingExternalReferenceCount==false))
  {
    m_Unregistering=true;

    int realReferenceCount = GetExternalReferenceCount()-1; //-1 because someone is trying to unregister us
    if(realReferenceCount<0)
      m_ExternalReferenceCount=realReferenceCount=0;

    if(realReferenceCount==0)
    {
      DataObjectPointerArray& outputs = const_cast<mitk::BaseProcess*>(this)->GetOutputs();
      //disconnect all outputs from us
      //size of outputs will not change until the very last output
      //is removed, because we remove from front to back
      unsigned int idx;
      for (idx = 0; idx < outputs.size(); ++idx)
      {
        const_cast<mitk::BaseProcess*>(this)->RemoveOutput(outputs[idx]);
      }
      //now the referenceCount should be one!
      int testReferenceCount=GetReferenceCount();
      if(testReferenceCount!=1)
      {
        itkWarningMacro(<<"Reference count of process object unexpectedly "
          << "not 1 before final unregister but " << testReferenceCount);
      }
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
//##ModelId=3E8600DD0072
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
  this->Register();
  Superclass::SetNthOutput(idx, output);
  this->UnRegister();
}

/**
* Adds an output to the first null position in the output list.
* Expands the list memory if necessary
*/
//##ModelId=3E8600DD00F4
void mitk::BaseProcess::AddOutput(itk::DataObject *output)
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
  unsigned int idx=0;

  output = dynamic_cast<mitk::BaseData*>(output);

  if (output)
  {
    dynamic_cast<mitk::BaseData*>(output)->ConnectSource(this, idx);
  }
#endif
  Superclass::AddOutput(output);

}
