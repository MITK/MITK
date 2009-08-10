/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkSimpleFastMutexLock.h,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkSCOPEDLOCK_H
#define __itkSCOPEDLOCK_H 




namespace itk
{

template <class M>
class ScopedLock
	/// A class that simplifies thread synchronization
	/// with a mutex.
	/// The constructor accepts a Mutex and locks it.
	/// The destructor unlocks the mutex.
{
public:
	inline ScopedLock( M& mutex ): _mutex( mutex )
	{
		_mutex.Lock();
    
	}
	inline ~ScopedLock()
	{
		_mutex.Unlock();
	}

private:
	M& _mutex;

	ScopedLock();
	ScopedLock( const ScopedLock& );
	ScopedLock& operator = ( const ScopedLock& );
};


} // namespace itk


#endif // __itkSCOPEDLOCK_H