#ifndef FILEREADER_H_HEADER_INCLUDED_C1E7E521
#define FILEREADER_H_HEADER_INCLUDED_C1E7E521

#include "mitkCommon.h"
#include "Reader.h"

namespace mitk {

//##ModelId=3E186F28009B
class FileReader : public Reader
{
  protected:
    //##ModelId=3E188F1F027E
    FileReader();

  
    //##ModelId=3E188F2800DC
    ~FileReader();

  public:
          
  mitkClassMacro(FileReader, Reader);
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Specify the file to load. This is forwarded to the IO instance. 
   * Either the FileName or FilePrefix plus pattern are used to read
   * files. */
  itkSetStringMacro(FileName);
  itkGetStringMacro(FileName);
  
  /** Specify file prefix for the image file(s). You should specify either
   * a FileName or FilePrefix. Use FilePrefix if the data is stored
   * in multiple files. */
  itkSetStringMacro(FilePrefix);
  itkGetStringMacro(FilePrefix);
  
  /** The sprintf format used to build filename from FilePrefix and number. */
  itkSetStringMacro(FilePattern);
  itkGetStringMacro(FilePattern);
  
protected:
    //##ModelId=3E186FC80253
  std::string m_FileName;
    //##ModelId=3E1873B3030A
  std::string m_FilePrefix;
    //##ModelId=3E1873B30346
  std::string m_FilePattern;

};

} // namespace mitk



#endif /* FILEREADER_H_HEADER_INCLUDED_C1E7E521 */
