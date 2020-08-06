# mckgui.tcl -- model checking and then justification

# Author: Venkat
# Author: Yifei Dong
# $Id: mck.tcl,v 1.4 2010-08-19 15:03:40 spyrosh Exp $


#load the XSB interface for TCL
source "$xmc_path/GUI/xsb_interface.tcl"

if {$tcl_platform(platform) == "unix"} {
    source "$xmc_path/GUI/mclistbox.tcl"
}

package require mclistbox 1.02
catch {namespace import mclistbox::*}

# ----------------------------------------------------------------------
global process_name formula_name
global node_num node_childs
global node_exist node_info node_pred node_truth
set node_num 1
set node_childs(0) 0
set node_exist(0) 0
set process_name ""
set formula_name ""

proc mck_input {} {
    global process_name formula_name

    toplevel .input
    wm title .input "Model Checking Input"
    
    # Add controls to dialog
#    set win [.input childsite]

    set w [frame .input.entries]
    pack $w -padx 4 -pady 4 -side top
    set pentry $w.pentry
    set pname $process_name
    iwidgets::entryfield $pentry -labeltext "Process:" -labelpos w \
	-textvariable pname
    pack $pentry -padx 4 -pady 4 -side top

    set fentry $w.fentry
    set fname $formula_name
    iwidgets::entryfield $fentry -labeltext "Formula:" -labelpos w \
	-command "mck_input_ok $pentry $fentry" \
	-textvariable fname
    pack $fentry -padx 4 -pady 4 -side top

    set btn [frame .input.buttons]
    pack $btn -side bottom -fill x -pady 2m
    button $btn.ok -text "OK" -command "mck_input_ok $pentry $fentry"
    button $btn.cancel -text "Cancel" -command { destroy .input }
    pack $btn.ok $btn.cancel -side left -expand 1

    focus [$pentry component entry]
}

proc mck_input_ok {pentry fentry} {
    global process_name formula_name
    set pname [$pentry get]
    set fname [$fentry get]
    if {$pname == ""} {
	focus [$pentry component entry]
    } elseif {$fname == ""} {
	focus [$fentry component entry]
    } else {
	destroy .input
	set process_name $pname
	set formula_name $fname
	mck_start
    }
}

global mckProofTree mckProcList

proc mck_start {} {
    global xsb_exec xmc_path
    global source_name process_name formula_name

    # disable "Start..." and enable "Stop..."
    .mb menuconfigure .mck.start -state disabled
    .mb menuconfigure .mck.stop -state normal

    # create model checking window
    toplevel .mck
    wm title .mck "$process_name |=?= $formula_name"
    wm iconname .mck $formula_name

    set w .mck
    # First, a message box shows the status of modelchecking
    iwidgets::messagebox $w.mb -labeltext "Model Checking Status" \
	-hscrollmode dynamic \
	-vscrollmode dynamic \
	-labelpos n -visibleitems 60x10

    pack $w.mb -padx 4 -pady 4 -fill both -expand yes

    $w.mb type add ERROR -background white -foreground red \
	-font "Helvetica -14"
	#-bell 1
    $w.mb type add INFO -background white -foreground black \
	-font "Helvetica -14"
    $w.mb type add ANSWER -background white -foreground blue \
	-font "Helvetica -14 bold"

    set btn [frame .mck.buttons]
    pack $btn -padx 4 -pady 4 -side bottom
    button $btn.cancel -text "Cancel" -command mck_stop
    pack $btn.cancel -padx 4 -pady 4 -side right -expand 1
    
    bind .mck <Escape> { mck_stop }

    # initialize XSB and XMC
    $w.mb issue "Initializing XMC ..." INFO
    update
    xsb_init "$xsb_exec" "\['$xmc_path/System/tcl_interface'\]"
    xsb_command "\['$xmc_path/System/xmc'\]"
    xsb_command {[gui]}

    # Compile the .xl file & start on XMC checking
    $w.mb issue "Compiling the source file ..." INFO
    update
    set xl_index [string last ".xl" $source_name]
    set xlfile [string range $source_name 0 [expr $xl_index -1]]
    xsb_query xmc_compile(\'$xlfile\',ErrorFile) result
    set errfile $result(1,1)
    if {$result(1,1) != "true"} {
	$w.mb issue "Error in source file: " ERROR
	# display error file
	set f [open $result(1,1) "r"]
	for {set line 1} {! [eof $f]} {incr line} {
            $w.mb issue [gets $f] INFO
	}
	close $f
	return
    }

    # start model checking
    $w.mb issue "Verifying process '$process_name'" INFO
    $w.mb issue "    with property '$formula_name' ..." INFO
    update
    xsb_query xmc_modelcheck($process_name,$formula_name,Truth) result
    set truth $result(1,1)

    if {$truth != "true" && $truth != "false"} {
	$w.mb issue "Error: $truth" ERROR
	return
    }

    # report answer
    $w.mb issue "The answer is [string toupper $truth]" ANSWER

    # Then justify
    $w.mb issue "You can now justify." INFO

    button $btn.justify -text "Justify" -command justify
    pack $btn.justify -padx 4 -pady 4 -side left -expand 1
    bind .mck <Return> { .mck invoke justify }
}

proc mck_stop {} {
    xsb_close
    catch { destroy .mck }
    catch { destroy .justify }
    # disable "Start..." and enable "Stop..."
    .mb menuconfigure .mck.start -state normal
    .mb menuconfigure .mck.stop -state disabled
}

# ----------------------------------------------------------------------
# The justifier

proc justify {} {
    global mckProofTree mckProcList
    global xmc_path

    # clean up window
    destroy .mck
    set w [toplevel .justify]
    wm title $w "Justifier"

    # load justifier's Prolog program
    xsb_command {[navigate,justify]}

    # initialize Prolog side of justifier
    xsb_command {begin_justification}

    set pw $w.p
    iwidgets::panedwindow $pw -orient horizontal \
	-width 640 -height 500
    pack $pw -side top -expand 1 -fill both

    # load icons
    set bitmap_path "$xmc_path/GUI"

    image create bitmap nodebmp  -file "$bitmap_path/node.bmp"
    image create bitmap openbmp  -file "$bitmap_path/open.bmp"
    image create bitmap closebmp -file "$bitmap_path/close.bmp"

    image create photo icontrue  -file "$bitmap_path/true.gif"
    image create photo iconfalse -file "$bitmap_path/false.gif"

    # The proof tree
    $pw add "proof"
    set mckProofTree [$pw childsite "proof"].prooftree
    iwidgets::hierarchy $mckProofTree -labeltext "Proof Tree" \
        -querycommand "query_node %n" \
	-alwaysquery 1 \
        -visibleitems 80x20 \
	-borderwidth 1 -tabs 12 \
	-hscrollmode dynamic \
	-vscrollmode dynamic \
	-sbwidth 10 -scrollmargin 0 \
        -nodeicon nodebmp \
        -openicon openbmp \
	-closedicon closebmp \
        -selectcommand "select_node %n" \
	-selectcolor "green" \
	-foreground "black" \
	-background "blue" \
	-font "Helvetica -14"
    pack $mckProofTree -side top -fill both -expand yes
    bind $w + expand_selection
    bind $w * expand_all

    # The process list
    $pw add "proc"
    set mckProcList [$pw childsite "proc"].proclist
    mclistbox::mclistbox $mckProcList \
        -bd 0 \
        -height 6 -width 80 \
	-columnrelief flat \
        -labelanchor w \
        -columnborderwidth 0 \
        -labelborderwidth 2 \
        -fillcolumn vars \
	-background white \
	-foreground red \
	-selectcommand highlight_bright \
	-selectmode single

    $mckProcList column add name -label "Process" -width 20
    $mckProcList column add line -label ""	-width 0
    $mckProcList column add char -label ""	-width 0
    $mckProcList column add vars -label "Variables" -width 60
    pack $mckProcList -side top -fill x -expand yes

    $pw fraction 70 30

    set btn [frame $w.buttons]
    pack $btn -padx 4 -pady 4 -side bottom
    button $btn.cancel -text "Cancel" -command mck_stop
    pack $btn.cancel -padx 4 -pady 4 -side right -expand 1 -fill x
    
    bind $w <Escape> { mck_stop }
}

# ----------------------------------------------------------------------
# basic node expansion, uses the prolog side to query all the children of
# a given node and returns the list to the hierarchy widget

proc query_node {node_id} {
    global process_name formula_name
    global mckProofTree 
    global node_num node_childs
    global node_exist node_info node_pred node_truth

    puts stdout "enter..."
    puts stdout $node_id

    if {$node_id == ""} {
	# root node: get models predicate
	xsb_query get_models($process_name,$formula_name,Models) result
	set models $result(1,1)
	xsb_query getroot($models,Id,Pred,Truth) childlist
#	xsb_query getroot(mck($process_name,$formula_name),Id,Pred,Truth) childlist
	set node_root 1
	set node_info($node_num) $childlist(1,1)
	set node_pred($node_num) $childlist(1,2)
	set node_truth($node_num) $childlist(1,3)
	set node_exist($childlist(1,1)) $node_num
	set childlist(1,1) $node_num
    } else {
	if [info exists node_childs($node_id)] {
	    set j [llength $node_childs($node_id)]
	    for {set i 0} {$i < $j} {incr i} {
#		set child_id $node_info([lindex $node_childs($node_id) $i])
#		xsb_query getroot($child_id,Id,Pred,Truth) childlist1
		set child_id [lindex $node_childs($node_id) $i]
		set childlist([expr $i + 1],1) $child_id
		set childlist([expr $i + 1],2) $node_pred($child_id)
		set childlist([expr $i + 1],3) $node_truth($child_id)
	    }
	    set childlist(ntuples) $j
	    
	} else {
	    set node_id1 $node_info($node_id)
	    xsb_query getchild($node_id1,Id,Pred,Truth) childlist
	    set node_childs($node_id) {}
	    puts stdout [format "entering %s" $node_id]
	    for {set i 1} {$i <= $childlist(ntuples)} {incr i} {
		incr node_num
		if [info exists node_exist($childlist($i,1))] {
		    set node_info($node_num) [format "anc(%s)" $childlist($i,1)]
		    set node_pred($node_num) [format "Loop: %s" $childlist($i,2)]
		} else {
		    set node_info($node_num) $childlist($i,1)
		    set node_exist($childlist($i,1)) 1
		    set node_pred($node_num) $childlist($i,2)
		}
		set childlist($i,1) $node_num
		set node_truth($node_num) $childlist($i,3)
		lappend node_childs($node_id) $node_num
		set childlist($i,1) $node_num
		puts stdout [format "info : %s" $node_info($node_num)]
		puts stdout [format "pred : %s" $node_pred($node_num)]
	    }
	}
    }

    set rlist {}
    for {set i 1} {$i <= $childlist(ntuples) } {incr i} {
	set child_id	$childlist($i,1)
	set child_pred	$childlist($i,2)
	set child_truth	$childlist($i,3)

	# display truth value by icon
	lappend rlist [list $child_id $child_pred {} "icon$child_truth" ]
    }

    # scroll down the proof tree
    # not really!
#    $mckProofTree component list yview moveto 1

    return $rlist
}

#------------------------------------------------------------------------

proc expand_all_subtree {node} {
    global mckProofTree mckProcList
    set children [query_node $node]

    foreach tagset $children {
	set uid [lindex $tagset 0]

	$mckProofTree expand $uid

	if {[query_node $uid] != {}} {
	    expand_all_subtree $uid
	}
    }
}

proc expand_selection {} {
    global mckProofTree

    expand_all_subtree [lindex [$mckProofTree selection get] 0]
}

#------------------------------------------------------------------------
# expand starting from the root
proc expand_all {} {
    expand_all_subtree ""
}

#------------------------------------------------------------------------

proc highlight_one {p color} {
    global src_area

    set line [lindex $p 1]
    set char [lindex $p 2]
    $src_area tag add "LINE$color" $line.0 [expr $line+1].0
    $src_area tag add "CHAR$color" $line.[expr $char-1] $line.$char
}

proc highlight_all {} {
    global mckProcList src_area

    # clear all existing highlights
    $src_area tag delete LINEdim CHARdim LINEbright CHARbright
    set_highlight_color

    # add new highlights
    set proclist [$mckProcList get 0 end]
    foreach process $proclist {
	highlight_one $process dim
    }
}

proc highlight_bright {{what ""}} {
    global mckProcList src_area

    if {$what != ""} {
	# clear current bright highlight
	$src_area tag delete LINEbright CHARbright
	set_highlight_color
	# bright new highlight
	set process [$mckProcList get $what]
	highlight_one $process bright
    }
}


proc select_node {node_id} {
    global mckProofTree mckProcList
    global node_info

    # set current selection & show it
    $mckProofTree selection clear
    $mckProofTree selection add $node_id

    # clear the process list
    catch { $mckProcList delete 0 end }

    # get the symbol table
    set node_id1 $node_info($node_id)
    xsb_query get_process_list($node_id1,Proc,Line,Char,Vars) result

    # fill the process list
    for {set x 1} {$x <= $result(ntuples)} {incr x } {
	set process	$result($x,1)
	set line	$result($x,2)
	set char	$result($x,3)
	set variables	[string range $result($x,4) 1 \
			 [expr [string length $result($x,4)] - 2]]

	$mckProcList insert end [list $process $line $char $variables]
    }

    # highlight the lines
    highlight_all
}
