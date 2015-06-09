Reader and Writer {#ReaderWriterPage}
=================

This page is work in progress and will introduce you to the IO-System of MITK.

[TOC]

## Introductory slides

Several Talks have been given on the IO-System. The following list should provide you with a good starting point.

- Introduction to MimeTypes: http://www.mitk.org/images/e/e8/MimeTypes.pdf (by Caspar J. Goch)
- Introduction to the IO-System: http://www.mitk.org/images/0/0b/Newio.pdf (by Keno März)
- IO-Streams and the new IO System: http://www.mitk.org/images/9/95/Streams.pdf (by Keno März)

## Quick start: Reading and writing files using IOUtil

mitk::IOUtil class provides convenience methods for loading data into a data
storage or just returning BaseData objects without user interaction. The mitk::IOUtil::Save()
and mitk::IOUtil::Load() methods cover the typical use cases and
automatically select the best matching mitk::IFileReader and mitk::IFileWriter
instance. In most cases, this is the easiest way to read or write a file.

     // load files directly into datastorage
    mitk::IOUtil::Load("/path/to/my/file.nrrd",*ds);

     // load basedata into local vector
    std::vector< mitk::Basedata::Pointer > basedatas;
     basedatas = mitk::IOUtil::Load("/path/to/my/file.nrrd");

     // write basedata to file (here: surface as PLY
     mitk::IOUtil::Save(mySurface, "/Save/surface/here.ply");

     // write basedata to file (here: surface as STL
     mitk::IOUtil::Save(mySurface, "/Save/surface/here.stl");

When reading a file using IOUtil, the IO-Framework  first determines the mime-type of the given file.
Afterwards, the best reader is selected internally, instantiated, and executed. The resulting BaseData Objects are returned to the developer as the method result.

## Quick start: Creating your own reader or writer

If you implement a new BaseData, usually matching readers and writers are required. The following guide will help you to quickly set up these up and get them working in MITK.

Create new classes for reader and writers. Optimally, place them in an extra IO-Module that is configured as autoload. This way, they are available to the application from the start. If you are working  with common data, the appropriate module is MitkIOExt. You can either extend mitk::AbstractFileIO, which will allow you to implement a class with reader and writer abilities, or you can extend mitk::AbstractFileReader or mitk::AbstractFileWriter specifically.

\imageMacro{reader_writer_classes.png,"",16}

Implement the given Methods. A good example on how to write a simple reader and writer is the mitkPointSetReaderService.cpp and mitkPointSetWriterService.cpp class, from which you can take implementation cues. The following is a simplified version of the header file:

     namespace mitk
     {

     class PointSetReaderService: public AbstractFileReader // 2) Extend the Abstract File Reader
     {
     public:

       PointSetReaderService();  // 3) Provide Constructor and Destructor
       virtual ~PointSetReaderService();

       // 4) Overwrite the Read Method as seen here
       using AbstractFileReader::Read;
       virtual std::vector< itk::SmartPointer<BaseData> > Read();

     private:

       // 5) Provide a clone method
       PointSetReaderService(const PointSetReaderService& other);
       virtual PointSetReaderService* Clone() const;
     };

     }

### Example
Follow these steps to implement a new Reader:

A) Create a new cpp and h file in an appropriate submodule. Usually, a reader or writer should be located in the same module as the BaseData derivate it reads/writes.

B) Extend AbstractFileReader . This is highly recommended because it enables integration of your Reader into the Registery. It will then automatically be used by the application to load this type of files.

C) Provide a constructor . It should contain a minimal amount of information and might look like this:

    mitk::PointSetReaderService::PointSetReaderService()
      : AbstractFileReader(CustomMimeType(IOMimeTypes::POINTSET_MIMETYPE_NAME()), "MITK Point Set Reader")
    {
        RegisterService();
    }

Note the call to the superclass constructor containing the MIME-type. You can either reuse an existent MIME type here or create your own MIME-type locally . Finally, register the service to make it available to MITK.

D) Provide a Clone Method: Readers are clones when the registry requires a new reader. Provide a clone method to accommodate for this. Use the mitkPointSetReaderService.cpp as a reference if necessary.

E) Instantiate it in the module activator. Open the module activator and make sure that the new Reader/Writer is instantiated and held somewhere in the code.  Also, unregister the reader/writer in the unload function if necessary.

## Reader/Writer Options

Options are a powerful concept to modify Reader/Writer behaviour and to automate user interaction with minimal development overhead. In principal, options are represented by a simple map and a few getter and setter functions:

    typedef std::map<std::string, us::Any> Options;

    virtual Options GetOptions();
    virtual void SetOptions(const Options& options);
    virtual us::Any GetOption(const std::string& name);
    virtual void SetOptions(const Options& options);

In its constructor, a reader or writer can set its options as a number map entries consisting of an human-readable option name and a default value. When a user tries to read/write a file via the interface, an options menu is generate from the map.

\imageMacro{io_options.png,"The options dialog shown by the raw reader",5}

The user can then modify these options, which are automatically set in the reader/writer. You can use raw-images to see this behaviour in action.

## Mime Types

The MimeType class provides meta-data about a specific data format.
Every mitk::IFileReader and mitk::IFileWriter instance must be associated
with exactly one mime-type via a service property.

\imageMacro{mimetypes.png,"",14}

Mime-type are used for categorizing data formats and creating filter
strings in file open and save dialogs. Hence they effectively control
the accessible set of data formats from a graphical user interface.
Minimally, mime-types should provide a name and a list of handled extensions in lower case.
Additionally, it is highly encouraged to set a category and a comment which will provide user-readable
strings for user interaction.

It is important to understand the difference between mitk::MimeType and mitk::CustomMimeType.
The former is an immutable stack object and can be pulled from mitk::MimeTypeProvider.
it should be used for all interaction with MimeTypes. mitk::CustomMimeType is the heap-object pendant
which is wrapped by mitk::MimeType and should exclusively be used for registration purposes, i.e. when you
register a new MimeType.

## Additional Capabilities

You can extend your reader writer with useful capabilities. All of these are optional
+ Priority: Reader use a ranking with Get- and SetRanking in order to signify how well they are suited to read a file. If several readers are able to read a file, the one with the highest Ranking level will be chosen.
+ ProgressCallbacks : Readers are executed in a thread automatically. If the reader implements callbacks, the progress bar will be more accurate during loading of files. Note: Progress callbacks are work in progress.

## Ranking strategies

Todo.

## Convenience classes

Developers usually do not interact with the service registry directly
to retrieve and select a matching mitk::IFileReader or mitk::IFileWriter instance.


### %QmitkIOUtil

The QmitkIOUtil class is a wrapper around mitk::IOUtil, providing
file open and save dialogues for selecting a file name from a within
a graphical user interface.

### FileReaderRegistry and FileWriterRegistry

Access to mitk::IFileReader and mitk::IFileWriter objects and their service references.

## Integrating external I/O mechanisms

The MITK I/O system integrates with several external I/O systems.

### ITK Reader and Writer

The I/O classes from ITK are automatically added to the service registry
of MITK. They can be transparently used via the mitk::IFileReader and mitk::IFileWriter
interfaces or the mitk::IOUtil methods.

### VTK Reader and Writer

VTK does not provide a mechanism to enumerate all available I/O classes.
Hence MITK provides manual integration of a specific set of VTK readers
and writers.

## Error handling and recovery

Todo.
