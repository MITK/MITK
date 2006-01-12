/*
 * Created on 05.07.2005
 */
package stateMachines;

import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.resource.ImageDescriptor;

import org.eclipse.gef.palette.CombinedTemplateCreationEntry;
import org.eclipse.gef.palette.ConnectionCreationToolEntry;
import org.eclipse.gef.palette.MarqueeToolEntry;
import org.eclipse.gef.palette.PaletteContainer;
import org.eclipse.gef.palette.PaletteDrawer;
import org.eclipse.gef.palette.PaletteGroup;
import org.eclipse.gef.palette.PaletteRoot;
import org.eclipse.gef.palette.PaletteSeparator;
import org.eclipse.gef.palette.PanningSelectionToolEntry;
import org.eclipse.gef.palette.ToolEntry;
import org.eclipse.gef.requests.CreationFactory;
import org.eclipse.gef.requests.SimpleFactory;
import org.eclipse.gef.ui.palette.FlyoutPaletteComposite.FlyoutPreferences;

import model.Connection;
import model.StartState;
import model.NormalState;
import model.EndState;


/**
 * Utility class that can create a GEF Palette.
 * 
 * @see #createPalette()
 * @author Daniel
 */
final class StateMachinesEditorPaletteFactory {

	/** Preference ID used to persist the palette location. */
	private static final String PALETTE_DOCK_LOCATION = "StateMachinesEditorPaletteFactory.Location";

	/** Preference ID used to persist the palette size. */
	private static final String PALETTE_SIZE = "StateMachinesEditorPaletteFactory.Size";

	/** Preference ID used to persist the flyout palette's state. */
	private static final String PALETTE_STATE = "StateMachinesEditorPaletteFactory.State";

	/** Create the "States" drawer. */
	private static PaletteContainer createStatesDrawer() {
		PaletteDrawer componentsDrawer = new PaletteDrawer("States");

		CombinedTemplateCreationEntry component = new CombinedTemplateCreationEntry(
				"Startstate", "Create a startstate", StartState.class,
				new SimpleFactory(StartState.class), ImageDescriptor
						.createFromFile(StateMachinesPlugin.class,
								"ellipse16.gif"), ImageDescriptor
						.createFromFile(StateMachinesPlugin.class,
								"ellipse24.gif"));
		componentsDrawer.add(component);

		component = new CombinedTemplateCreationEntry("Normalstate",
				"Create a state", NormalState.class, new SimpleFactory(
						NormalState.class), ImageDescriptor
						.createFromFile(StateMachinesPlugin.class,
								"rectangle16.gif"), ImageDescriptor
						.createFromFile(StateMachinesPlugin.class,
								"rectangle24.gif"));
		componentsDrawer.add(component);

		component = new CombinedTemplateCreationEntry("Endstate",
				"Create an endstate", EndState.class,
				new SimpleFactory(EndState.class), ImageDescriptor
						.createFromFile(StateMachinesPlugin.class,
								"rectangle16.gif"), ImageDescriptor
						.createFromFile(StateMachinesPlugin.class,
								"rectangle24.gif"));
		componentsDrawer.add(component);

		return componentsDrawer;
	}

	/**
	 * Creates the PaletteRoot and adds all palette elements. Use this factory
	 * method to create a new palette for your graphical editor.
	 * 
	 * @return a new PaletteRoot
	 */
	static PaletteRoot createPalette() {
		PaletteRoot palette = new PaletteRoot();
		palette.add(createToolsGroup(palette));
		palette.add(createStatesDrawer());
		return palette;
	}

	/**
	 * Return a FlyoutPreferences instance used to save/load the preferences of
	 * a flyout palette.
	 */
	static FlyoutPreferences createPalettePreferences() {
		return new FlyoutPreferences() {
			private IPreferenceStore getPreferenceStore() {
				return StateMachinesPlugin.getDefault().getPreferenceStore();
			}

			public int getDockLocation() {
				return getPreferenceStore().getInt(PALETTE_DOCK_LOCATION);
			}

			public int getPaletteState() {
				return getPreferenceStore().getInt(PALETTE_STATE);
			}

			public int getPaletteWidth() {
				return getPreferenceStore().getInt(PALETTE_SIZE);
			}

			public void setDockLocation(int location) {
				getPreferenceStore().setValue(PALETTE_DOCK_LOCATION, location);
			}

			public void setPaletteState(int state) {
				getPreferenceStore().setValue(PALETTE_STATE, state);
			}

			public void setPaletteWidth(int width) {
				getPreferenceStore().setValue(PALETTE_SIZE, width);
			}
		};
	}

	/** Create the "Tools" group. */
	private static PaletteContainer createToolsGroup(PaletteRoot palette) {
		PaletteGroup toolGroup = new PaletteGroup("Tools");

		// Add a selection tool to the group
		ToolEntry tool = new PanningSelectionToolEntry();
		toolGroup.add(tool);
		palette.setDefaultEntry(tool);

		// Add a marquee tool to the group
		toolGroup.add(new MarqueeToolEntry());

		// Add a (unnamed) separator to the group
		toolGroup.add(new PaletteSeparator());

		// Add (solid-line) connection tool
		tool = new ConnectionCreationToolEntry("Transition",
				"Create a transition between states", new CreationFactory() {
					public Object getNewObject() {
						return null;
					}

					// see StateMachinesEditPart#createEditPolicies()
					// this is abused to transmit the desired line style
					public Object getObjectType() {
						return Connection.SOLID_CONNECTION;
					}
				}, ImageDescriptor.createFromFile(StateMachinesPlugin.class,
						"connection_s16.gif"), ImageDescriptor.createFromFile(
						StateMachinesPlugin.class, "connection_s24.gif"));
		toolGroup.add(tool);

		return toolGroup;
	}

	/** Utility class. */
	private StateMachinesEditorPaletteFactory() {
		// Utility class
	}

}