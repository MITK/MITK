package stateMachines;

import org.eclipse.ui.IWorkbenchPreferenceConstants;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.plugin.AbstractUIPlugin;

/**
 * The main plugin class to be used in the desktop.
 */
public class StateMachinesPlugin extends AbstractUIPlugin implements IWorkbenchPreferenceConstants{

	/** Single plugin instance. */
	private static StateMachinesPlugin singleton;

	/**
	 * Returns the shared plugin instance.
	 */
	public static StateMachinesPlugin getDefault() {
		// all editors should be closed on exit eclipse
		PlatformUI.getPreferenceStore().putValue(CLOSE_EDITORS_ON_EXIT, "true");
		return singleton;
	}

	/**
	 * The constructor.
	 */
	public StateMachinesPlugin() {
		if (singleton == null) {
			singleton = this;
		}
	}
}