### xmc.tcl --- XMC GUI in Tcl/Tk

### Author: Yifei Dong
### $Id: xmc.tcl,v 1.3 2010-08-19 15:03:40 spyrosh Exp $

package require Iwidgets 3.0

global xmc_path xsb_exec
global source_name

catch {
    if { $env(XMC_PATH) != "" } {
	set xmc_path  $env(XMC_PATH)
    }
} 

catch {
    if { $env(XSB_EXEC) != "" } {
	set xsb_exec  $env(XSB_EXEC)
    }
}

set appname "XMC"
wm title . $appname
source $xmc_path/GUI/mck.tcl

# create menubar
iwidgets::menubar .mb -helpvariable helpVar -menubuttons {
    menubutton file -text "File" -menu {
        options -tearoff false

        command open -label "Open..." \
	    -helpstr "Open an existing file" \
	    -command {fileDialog . .src}

#         command new -label "New" \
# 	    -helpstr "Open new document" \
# 	    -command {}

#         command close -label "Close" \
# 	    -helpstr "Close current document" \
# 	    -command {}

        command save -label "Save" \
	    -helpstr "Save a file" \
	    -command fileSave \
	    -state disabled

        separator sep1

        command exit -label "Exit" \
	    -helpstr "Exit application" \
	    -command {exit} \
    }

#     menubutton edit -text "Edit" -menu {
#         options -tearoff false

#         command undo -label "Undo" -underline 0 \
#                 -helpstr "Undo last command" \
#                 -command {puts "selected: Undo"}

#         separator sep2

#         command cut -label "Cut" -underline 1 \
#                 -helpstr "Cut selection to clipboard" \
#                 -command {puts CUT}

#         command copy -label "Copy" -underline 1 \
#                 -helpstr "Copy selection to clipboard" \
#                 -command {puts "selected: Copy"}

#         command paste -label "Paste" -underline 0 \
#                 -helpstr "Paste clipboard contents into document" \
#                 -command {puts "selected: Paste"}
#     }

    menubutton mck -text "Model Check" -menu {
        options -tearoff false

        command start -label "Start..." \
	    -helpstr "Start model checking" \
	    -command {mck_input} \
	    -state disabled

        command stop -label "Stop..." \
	    -helpstr "Stop model checking" \
	    -command {mck_stop} \
	    -state disabled
    }
}
pack .mb -fill x

# create source text area

set src_area .src
iwidgets::scrolledtext $src_area -visibleitems 80x35 \
    -wrap none \
    -textfont "Courier -14" \
    -textbackground whitesmoke -foreground black \
    -hscrollmode dynamic -vscrollmode dynamic \
    -state disabled
pack $src_area -fill both -expand yes
#.src tag configure LINE -foreground blue

# highlight colors
proc set_highlight_color {} {
    .src tag configure LINEdim -background orange
    .src tag configure CHARdim -foreground blue
    .src tag configure LINEbright -background yellow
    .src tag configure CHARbright -foreground red
}

# create status bar
label .help -anchor w -textvariable helpVar -width 40 -relief sunken
pack .help -fill x


# Popup Open file dialog code and get file name
#
global last_dir
set last_dir [pwd]
proc fileDialog {toplevel textwidget} {
    global appname
    global last_dir
    global source_name
    
    #   Type names	Extension(s)	Mac File Type(s)
    #   ------------------------------------------------
    set types {
	{"XL files"	{.xl}		}
	{"Text files"	{.txt .TXT}	}
	{"All files"	*		}
    }

    set filepath [tk_getOpenFile \
        -initialdir $last_dir \
        -filetypes $types -parent $toplevel]
    
    if {$filepath != ""} {

	set last_dir [file dirname $filepath]

	# turn on editing
	.src configure -state normal
	.src clear

#        .src import $filepath
	# read file and add line number
	set f [open $filepath "r"]
	for {set line 1} {! [eof $f]} {incr line} {
#            .src insert end $line LINE
#            .src insert end "\t"
            .src insert end [gets $f]
            .src insert end "\n"
	}
	close $f
	# turn off editing
	.src configure -state disabled

	.mb menuconfigure .mck.start -state normal
#	.mb menuconfigure .file.save -state normal
	set source_name $filepath
	wm title . "$appname -- $source_name"
    }
}

proc fileSave {} {
    global source_name
    .src export $source_name
}
