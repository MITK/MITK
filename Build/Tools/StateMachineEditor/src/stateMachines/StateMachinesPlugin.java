package stateMachines;

import org.eclipse.ui.plugin.AbstractUIPlugin;

/**
 * The main plugin class to be used in the desktop.
 */
public class StateMachinesPlugin extends AbstractUIPlugin {

	/** Single plugin instance. */
	private static StateMachinesPlugin singleton;

	/**
	 * Returns the shared plugin instance.
	 */
	public static StateMachinesPlugin getDefault() {
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