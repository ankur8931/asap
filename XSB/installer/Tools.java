/****************************************************************************
			    XSB Installation
File name: 			Tools.java
Author(s): 			Dongli Zhang
Brief description: 	This is the class including some static functions we implemented
****************************************************************************/


import java.io.BufferedReader;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Enumeration;
import java.util.Properties;


public class Tools {
	
    //This function runs the command and get the result
    public static String runCommand(String command)
    {
        String returnString = "";
        String lineString=null;
        Process process = null;
	
        try {
            process = Runtime.getRuntime().exec(command);
            process.waitFor();
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            
            while ((lineString = bufferedReader.readLine())!= null)
            {
                returnString=returnString+lineString+"\n";
            }

        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return returnString;
    }
	
	//This function checks result from the run of command
	public static int checkSoftFromResult(String output, String software)
    {
        if(output.toLowerCase().contains(software.toLowerCase())) {
            return 1;
        }
	
        return 0;
    }
	
    //This function returns the name of operating system
    public static String checkOSType()
    {
	String osName= System.getProperty("os.name");
	String arch = System.getProperty("os.arch");
	
	if(osName.toLowerCase().contains("windows")) {
	    if(arch.toLowerCase().contains("x86")) {
		return "Windows 32-bit";
	    }
	    else
		{
		    return "Windows 64-bit";
		}
        }
	
	
	if(osName.toLowerCase().contains("linux")) {
	    String resultVersion=null;
            resultVersion=runCommand("cat /proc/version");
	    Properties props = new Properties();
	    try {
		InputStream in = Object.class.getResourceAsStream("/system.properties");
		props.load(in);
		
		Enumeration list = props.propertyNames();
		while(list.hasMoreElements()) {
		    String key =(String)list.nextElement();
		    String value = props.getProperty(key);
		    
		    if(resultVersion.toLowerCase().contains(key)) {
			return value;
		    }
		}
	    } catch (Exception e) {
		e.printStackTrace();	
	    }
        }
	return null;
    }
	
    //This function checks whether a file existed. We use it to check javac, jni.h, xsb.exe, etc.
    public static int fileExist(String path)
    {
	File file = new File(path);
	if (file.exists()) {
	    return 1;
	}
	else {
	    return 0;
	}
    }
}
