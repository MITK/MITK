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


#ifndef FileReaderManager_H_HEADER_INCLUDED_C1E7E521
#define FileReaderManager_H_HEADER_INCLUDED_C1E7E521

#include <MitkExports.h>
#include <mitkBaseData.h>
#include <mitkFileReaderInterface.h>

// Microservices
#include <mitkServiceReference.h>



namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class MITK_CORE_EXPORT FileReaderManager
{
  public:

    /**
    * Reads the file located at <code>path</code> and returns the
    * contents as a DataNode.
    *
    * This method will select the best available reader in the service
    * registry for the task.
    *
    * UnsupportedFileException: If no compatible reader was found
    * FileNotFoundException: If no file was found at <code>path</code>
    * FileReadException: If the selected reader failed to read the file
    **/

    static mitk::BaseData::Pointer Read(std::string path);

    template <class T>
    static T* Read(std::string path);
   // {
   //   if (this->m_Controls->m_ServiceList->currentRow()==-1) return NULL;
   //   mitk::ServiceReference ref = GetServiceForListItem( this->m_Controls->m_ServiceList->currentItem() );
   //   return ( m_Context->GetService<T>(ref) );
   /// }

    /**
    * Returns a compatible Reader to the given file extension
    **/
    static mitk::FileReaderInterface* GetReader(std::string extension);

    static mitk::FileReaderInterface* GetReader(std::string extension, std::list<std::string> options );

    static std::list <mitk::FileReaderInterface*> GetReaders(std::string extension);

    //static std::list <mitk::FileReaderInterface*> GetReaders(std::string extension, std::list<std::string> options );

    static bool CompareServiceRef(mitk::ServiceReference first, mitk::ServiceReference second);

protected:
    //FileReaderManager();
    //virtual ~FileReaderManager();

    static std::list< mitk::ServiceReference > GetReaderList(std::string extension);

    static bool ReaderSupportsOptions(mitk::FileReaderInterface* reader, std::list<std::string> options );

};
} // namespace mitk
#endif /* FileReaderManager_H_HEADER_INCLUDED_C1E7E521 */
