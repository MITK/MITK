#ifndef FILEWRITERWITHINFORMATION_H_INCLUDED
#define FILEWRITERWITHINFORMATION_H_INCLUDED

#include "mitkFileWriter.h"
#include "mitkBaseData.h"

namespace mitk {
/**
 * \brief Interface for FileWriters with extra information.
 * Should be merged into FileWriter. 
*/
class FileWriterWithInformation : public FileWriter {
  public:
    mitkClassMacro(FileWriterWithInformation,FileWriter);

    virtual const char * GetDefaultFilename() = 0;
    virtual const char * GetFileDialogPattern() = 0;
    virtual bool CanWrite(BaseData::Pointer data) = 0;  
    virtual void DoWrite(BaseData::Pointer data) = 0;
};
}
#endif

