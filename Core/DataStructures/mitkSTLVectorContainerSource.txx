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

#ifndef _MITK_STL_VECTOR_CONTAINER_SOURCE_TXX_
#define _MITK_STL_VECTOR_CONTAINER_SOURCE_TXX_

#include "mitkSTLVectorContainerSource.h"

template <typename T>
mitk::STLVectorContainerSource<T>::STLVectorContainerSource()
{
    OutputTypePointer output = static_cast<OutputType*>( this->MakeOutput( 0 ).GetPointer() );

    assert( output.GetPointer() != NULL );

    this->ProcessObject::SetNumberOfRequiredOutputs( 1 );
    this->ProcessObject::SetNthOutput( 0, output.GetPointer() );
}


template <typename T>
mitk::STLVectorContainerSource<T>::~STLVectorContainerSource()
{}




template <typename T>
typename mitk::STLVectorContainerSource<T>::DataObjectPointer
mitk::STLVectorContainerSource<T>::MakeOutput ( unsigned int )
{
    return OutputType::New().GetPointer();
}




template <typename T>
void
mitk::STLVectorContainerSource<T>::SetOutput( OutputType* output )
{
    itkWarningMacro( << "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
    this->SetNthOutput( 0, output );
}




template <typename T>
typename mitk::STLVectorContainerSource<T>::OutputType*
mitk::STLVectorContainerSource<T>::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }

    if ( static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) ) == NULL )
        itkWarningMacro(<< "Output is NULL!" );
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) );
}




template <typename T>
typename mitk::STLVectorContainerSource<T>::OutputType*
mitk::STLVectorContainerSource<T>::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( idx ) );
}




template <typename T>
void
mitk::STLVectorContainerSource<T>::GenerateInputRequestedRegion()
{
    this->ProcessObject::GenerateInputRequestedRegion();
}




template <typename T>
void
mitk::STLVectorContainerSource<T>::GraftOutput( OutputType* graft )
{
    OutputType * output = this->GetOutput();

    if ( output && graft )
    {
        // copy the meta-information
        output->CopyInformation( graft );
    }
}


#endif
