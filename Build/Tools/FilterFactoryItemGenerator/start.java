import java.io.IOException;


/**
 * this class first opens CopyTemplate.class
 * to write a new FilterFactoryFilterItem from a template file
 * and then opens UpdateFiles.class to insert lines to the files specified there.
 */
public class start {

  String Title, mitkPath, xmlPath, inputClass, inputDim, outputClass, outputDim;


  public start(String Title, String mitkPath, String xmlPath,
      String inputClass, String inputDim, String outputClass,
      String outputDim)
  {
    this.Title = Title;
    this.mitkPath = mitkPath;
    this.xmlPath = xmlPath;
    this.inputClass = inputClass;
    this.inputDim = inputDim;
    this.outputClass = outputClass;
    this.outputDim = outputDim;
  }

  public void run()
  throws IOException
  {
    // copy template and replace title
    CopyTemplate ct = new CopyTemplate(Title, mitkPath);
    ct.run();

    // update various files
    UpdateFiles uq = new UpdateFiles(Title, mitkPath, xmlPath, inputClass,
        inputDim, outputClass, outputDim);
    uq.run();
  }
}
