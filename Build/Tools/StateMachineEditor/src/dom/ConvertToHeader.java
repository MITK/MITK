package dom;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkbenchWindowActionDelegate;
import org.eclipse.ui.PlatformUI;

public class ConvertToHeader implements IWorkbenchWindowActionDelegate {

	/**
	 * constructor to convert the xml file with the constants to a C++ header
	 * used when new actions or events are created
	 * @param filename the filename from the constants xml file
	 * @throws FileNotFoundException
	 * @throws TransformerException
	 */
	public ConvertToHeader(File filename) throws FileNotFoundException, TransformerException {
		// change filename to match xsl file
		String file = filename.getParentFile() + File.separator + "mitkEventAndActionConstants.xsl";
		TransformerFactory tFactory = TransformerFactory.newInstance();
		Transformer transformer = tFactory.newTransformer(new StreamSource(file));
		transformer.transform(new StreamSource(filename), new StreamResult(new FileOutputStream(filename.getParentFile() + File.separator + "mitkInteractionConst_h.h")));
	}

	/**
	 * constructor to convert the xml file with the constants to a C++ header
	 * used when the button in the workspace is pressed
	 */
	public ConvertToHeader() {
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IActionDelegate#run(org.eclipse.jface.action.IAction)
	 */
	public void run(IAction action) {
		// get the constants xml file
		Shell shell = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell();
		FileDialog fileDialog = new FileDialog(shell);
		fileDialog.setFilterExtensions(new String[] {"*.xml"});
		fileDialog.open();
		if (!(fileDialog.getFileName().equals(""))) {
			String file = fileDialog.getFilterPath();
			String file1 = fileDialog.getFileName();
			String file2 = file + File.separator + file1;
			File xmlfilename = new File(file2);
			File xslfilename = new File(xmlfilename.getParent() + File.separator + "mitkEventAndActionConstants.xsl");
			TransformerFactory tFactory = TransformerFactory.newInstance();
			Transformer transformer;
			try {
				transformer = tFactory.newTransformer(new StreamSource(xslfilename));
				transformer.transform(new StreamSource(xmlfilename), new StreamResult(new FileOutputStream(xmlfilename.getParentFile() + File.separator + "mitkInteractionConst_h.h")));
			} catch (TransformerConfigurationException e) {
				e.printStackTrace();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (TransformerException e) {
				e.printStackTrace();
			}
		}
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchWindowActionDelegate#dispose()
	 */
	public void dispose() {	
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchWindowActionDelegate#init(org.eclipse.ui.IWorkbenchWindow)
	 */
	public void init(IWorkbenchWindow window) {
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IActionDelegate#selectionChanged(org.eclipse.jface.action.IAction, org.eclipse.jface.viewers.ISelection)
	 */
	public void selectionChanged(IAction action, ISelection selection) {	
	}
}
