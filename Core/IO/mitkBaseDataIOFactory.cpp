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

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "mitkBaseDataIOFactory.h"
#include "mitkBaseProcess.h"
#include "mitkIOAdapter.h"
//#include "mitkConfig.h"

#include "mitkPicFileReaderIOFactory.h"
#include "mitkParRecFileReaderIOFactory.h"
#include "mitkSTLFileReaderIOFactory.h"

//#ifdef MBI_INTERNAL
//#include "mitkVesselTreeFileReaderIOFactory.h"
//#endif // MBI_INTERNAL

#include "itkMutexLock.h"
#include "itkMutexLockHolder.h"

namespace mitk
{

BaseData::Pointer BaseDataIOFactory::CreateBaseDataIO(const std::string path, const std::string filePrefix, const std::string filePattern, FileModeType mode)
{

  RegisterBuiltInFactories();
  std::list<IOAdapterBase::Pointer> possibleIOAdapter;
  std::list<LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkIOAdapter");
  for(std::list<LightObject::Pointer>::iterator i = allobjects.begin();
    i != allobjects.end(); ++i)
  {
    IOAdapterBase* io = dynamic_cast<IOAdapterBase*>(i->GetPointer());
    if(io)
    {
      possibleIOAdapter.push_back(io);
    }
    else
    {
      std::cerr << "Error BaseDataIO factory did not return an IOAdapterBase: "
        << (*i)->GetNameOfClass() 
        << std::endl;
    }
  }
  for(std::list<IOAdapterBase::Pointer>::iterator k = possibleIOAdapter.begin();
    k != possibleIOAdapter.end(); ++k)
  { 
    if( mode == ReadMode )
    {
      if((*k)->CanReadFile(path, filePrefix, filePattern))
      {
        BaseProcess::Pointer ioObject = (*k)->CreateIOProcessObject(path, filePrefix, filePattern);
        ioObject->Update();
        return dynamic_cast<BaseData*>(ioObject->GetOutputs()[0].GetPointer());
      }
    }
    //else if( mode == WriteMode )
    //{
    //  if((*k)->CanWriteFile(path))
    //  {
    //    return *k;
    //  }

    //}
  }
  return 0;
}

void BaseDataIOFactory::RegisterBuiltInFactories()
{
  static bool firstTime = true;

  static itk::SimpleMutexLock mutex;
  {
    // This helper class makes sure the Mutex is unlocked 
    // in the event an exception is thrown.
    itk::MutexLockHolder<itk::SimpleMutexLock> mutexHolder( mutex );
    if( firstTime )
    {
      itk::ObjectFactoryBase::RegisterFactory( PicFileReaderIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( ParRecFileReaderIOFactory::New() );
      itk::ObjectFactoryBase::RegisterFactory( STLFileReaderIOFactory::New() );
//#ifdef MBI_INTERNAL
      //itk::ObjectFactoryBase::RegisterFactory( VesselTreeFileReaderIOFactory::New() );
//#endif // MBI_INTERNAL
      //ObjectFactoryBase::RegisterFactory( VTKBaseDataIOFactory::New() );
      //ObjectFactoryBase::RegisterFactory( GiplBaseDataIOFactory::New() );
      //ObjectFactoryBase::RegisterFactory( BioRadBaseDataIOFactory::New() );
      //ObjectFactoryBase::RegisterFactory( LSMBaseDataIOFactory::New()); //should be before TIFF
      //ObjectFactoryBase::RegisterFactory( NiftiBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( AnalyzeBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( StimulateBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( JPEGBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( TIFFBaseDataIOFactory::New());
      //ObjectFactoryBase::RegisterFactory( NrrdBaseDataIOFactory::New() );
      //ObjectFactoryBase::RegisterFactory( BMPBaseDataIOFactory::New() );
      //ObjectFactoryBase::RegisterFactory( DICOMImageIO2Factory::New() );
      firstTime = false;
    }
  }

}

} // end namespace itk
