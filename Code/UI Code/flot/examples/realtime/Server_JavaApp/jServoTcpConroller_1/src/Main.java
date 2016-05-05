
















/**
 *
 * @author secakmak
 */

import com.l2fprod.gui.plaf.skin.Skin;
import com.l2fprod.gui.plaf.skin.SkinLookAndFeel;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.MissingResourceException;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.UIManager;

public class Main 
{
    Interface developerScreen;
    
    //---------------MAC-GUI--------------------------------------------------------------
    public static final String SYSTEM_LOOK_AND_FEEL_NAME = "Normal";
    public static String TRANSLATIONS_PATH = "translations/";
    public static String THEMES_PATH = "themes/";
    public static final Locale DEFAULT_LOCALE = new Locale("en", "GB");
    protected static Map translations = new HashMap();


    public static InputStream loadTheme(String themeName) throws MissingResourceException, IOException
    {
        InputStream is = null;
        try
        {
                is = loadResource(themeName);
        } catch (MissingResourceException e1) {
                try {
                        is = loadResource(themeName + ".zip");
                } catch (MissingResourceException e2) {
                        try {
                                is = loadResource(THEMES_PATH + themeName);
                        } catch (MissingResourceException e3) {
                                is = loadResource(THEMES_PATH + themeName + ".zip");
                        }
                }
        }
        return is;
    }

    protected static InputStream loadResource(String resource) throws MissingResourceException, IOException
    {

        Thread thread = Thread.currentThread();
        ClassLoader cLoader = thread.getContextClassLoader();
        URL url = cLoader.getResource(resource);
        if (url == null) {
                throw new MissingResourceException("Unable to find resource '" + resource + "'.", resource, resource);
        }
        try {
                InputStream is = url.openStream();
                return is;
        } catch (IOException e) {
                throw new MissingResourceException("Unable to load resource '" + resource + "' (IOException).", resource,
                                resource);
        }
    }

    //----------------------------------------------------------------------------------------
     
    public Main()
    {

        
        try
	{
            String lookAndFeelName = "tigerthemepack"; //Normal
            JFrame.setDefaultLookAndFeelDecorated(true);
            JDialog.setDefaultLookAndFeelDecorated(true);
            if (SYSTEM_LOOK_AND_FEEL_NAME.equals(lookAndFeelName))
            {
                    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            }
            else
            {
                    Skin theSkinToUse = SkinLookAndFeel.loadThemePack(loadTheme(lookAndFeelName));
                    SkinLookAndFeel.setSkin(theSkinToUse);
                    SkinLookAndFeel lookAndFeel = new SkinLookAndFeel();
                    UIManager.setLookAndFeel(lookAndFeel);
            }
	}
	catch (Exception e)
	{
            e.printStackTrace();
	}
        
        if(developerScreen == null)
        {
                developerScreen = new Interface(new JFrame(),true);
                developerScreen.setTitle("IoT Servo Controller");
                Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
                developerScreen.setLocation((screen.width - developerScreen.getWidth()) / 2,(screen.height - developerScreen.getHeight()) / 2);
                developerScreen.setVisible(true);
        }
                 
    }
    
    public static void main(String args[])
    {
        new Main();
    }
}
