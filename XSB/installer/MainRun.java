/****************************************************************************
			      XSB Installation
File name: 			MainRun.java
Author(s): 			Dongli Zhang
Brief description: 	This is the class including the main function.
			It first decides the system type and then opens
			the corresponding frame.
****************************************************************************/


import javax.swing.JOptionPane;

public class MainRun {

    public static void main(String[] args) {
	//get the name of Operating System
	String osType=Tools.checkOSType();
	
	//If the system is not in the properties list
	if(osType==null) {
	    JOptionPane.showMessageDialog(null, "This installer does not support your operating system.  Please try command line installation.");
	    System.exit(0);
	}
		
	String message="We have determined that your operating system is "+osType+".  Please click OK if this is correct.\n";	
	int contValue;
	contValue=JOptionPane.showConfirmDialog(null, message,"Confirm your system",JOptionPane.OK_CANCEL_OPTION);

	if(contValue==JOptionPane.CANCEL_OPTION) {
	    System.exit(0);
	}
		
	if(!osType.toLowerCase().contains("windows 32-bit") && !osType.toLowerCase().contains("windows 64-bit")) {
	    //Open the frame for Linux
	    LinuxFrame linuxFrame = new LinuxFrame(osType);
	}
	else {
	    //Open the frame for Windows
	    WindowsFrame windowsFrame = new WindowsFrame(osType);
	}
    }
}
