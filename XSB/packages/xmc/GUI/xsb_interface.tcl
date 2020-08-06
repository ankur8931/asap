#  !/usr/shareware/bin/tclsh 
#
# internal procedure, to read to next prompt
#

global xsb_trace xsb_set_list

set xsb_trace 1
set xsb_tflag 0
set xsb_set_list ""

proc gets_xsb {where aline} {
    global xsb_proc xsb_trace
    upvar $aline line
    gets $xsb_proc line
    if {$xsb_trace && $line != ""} { puts "$where received: $line" }
}

proc xsb_readtoprompt {} {
    global xsb_proc xsb_trace
    set ans "error"
    # gets $xsb_proc line
    # if {$xsb_trace} {puts "received: $line"}
    gets_xsb xsb line
    while {$line != "| ?- "} {
	if {[string range $line 0 4] == "|tcl-"} {
	    set cmd [string range $line 5 end]
	    if {$xsb_trace} {puts "evaling: $cmd"}
	    eval $cmd
	    update idletasks
	} elseif {$line == ":"} {
	    puts $xsb_proc "" ; flush $xsb_proc
	    
	} elseif {($line == "yes") || ($line == "no")} {
	    set ans $line
	}
	# gets $xsb_proc line
	# if {$xsb_trace} {puts "received: $line"}
	gets_xsb xsb line
    }
    return $ans
}
# 
# initializes Prolog
proc xsb_init {prologcmd initcmd} {
    global xsb_proc xsb_trace
    #	if {[info exists xsb_proc]} {return 0}
    if {[info exists xsb_trace]} {} else {set xsb_trace 0}
    set xsb_proc [open "|$prologcmd" r+]
    set prompt [read $xsb_proc 5]
    while {$prompt != "| ?- "} {
	# gets $xsb_proc prompt
	gets_xsb xsb prompt
	set prompt [read $xsb_proc 5]
    }
    xsb_command $initcmd
    return $xsb_proc
}

# shut down Prolog
proc xsb_close {} {
    global xsb_proc
    catch {close $xsb_proc}
}

#
# Sends a command to Prolog and returns `yes' or `no', as the command does.
# Everything returned, except tcl| commands, and yes or no, is ignored.
#
proc xsb_command command {
	xsb_tcl_send
	xsb_command0 $command
}

proc xsb_command0 command {
	global xsb_proc xsb_trace
	if !{[string match "*\." $command]} {
		set command [format "%s." $command]
	}
	if {$xsb_trace} {puts "xsb_command sending: $command"}
	puts $xsb_proc $command
	flush $xsb_proc
	return [xsb_readtoprompt]
}
#
# reads from prolog to get a relation and puts it into an array
#
proc xsb_get_relation {line rel {numresult 0}} {
    global xsb_proc xsb_trace 
    upvar $rel relname
    while {$line == ""} {
	# if {$xsb_trace} {puts "xsb_query received: $line"}
	# gets $xsb_proc line
	gets_xsb query line
    }
#    if {[info exists $relname]} {unset $relname}
    catch { unset relname }
    for {set j 1} {($line != "no") && ($line != "yes") &&
	($numresult == 0 || $j-1 != $numresult)} {incr j} {
	for {set i 1} {$line != ":"} {incr i} {
	    regexp {^([^ ]+) = (.*)$} $line all varn line
	    if {$j == 1} { set relname(0,$i) $varn }
	    set relname($j,$i) $line
	    # gets $xsb_proc line
	    gets_xsb query line
	    # if {$xsb_trace} {puts "xsb_query received: $line"}
	}
	puts $xsb_proc ";" ; flush $xsb_proc
	# gets $xsb_proc line
	gets_xsb query line
	while {$line == ""} {
	    # if {$xsb_trace} {puts "xsb_query received: $line"}
	    # gets $xsb_proc line
	    gets_xsb query line
	}
	# if {$xsb_trace} {puts "xsb_query received: $line"}
    }
    xsb_readtoprompt
    set relname(ntuples) [expr $j-1]
    if {$j > 1} {
	set relname(nfields) [expr $i-1]
    } else {
	set relname(nfields) 0
    }
    return no
}
#
# Sends a query to Prolog and retrieves the answers and puts them in a
# global variable (an array) whose name is given as the second argument.
# rel(i,j) has the ith field of the jth tuple returned.
# rel(nfields) has the number of fields.
# rel(ntuples) has the number of tuples
#
proc xsb_query {query rel {numresult 0}} {
    global xsb_proc xsb_trace
    upvar $rel relname
    xsb_tcl_send
    if !{[string match "*\." $query]} {
	set query [format "%s." $query]
    }
    if {$xsb_trace} {puts "xsb_query sending: $query"}
    puts $xsb_proc $query
    flush $xsb_proc
    # gets $xsb_proc line
    gets_xsb query line
    # if {$xsb_trace} {puts "xsb_query received: $line"}
    if {$line == "*** syntax error ***"} {
	while {$line != ""} {
	    # gets $xsb_proc line
	    gets_xsb query line
	    # if {$xsb_trace} {puts "xsb_query received: $line"}
	}
	return error
    }
    while {[string range $line 0 4] == "|tcl-"} {
	if {$xsb_trace} {puts "xsb_query evaling: $line"}
	eval [string range $line 5 end]
	# gets $xsb_proc line
	gets_xsb query line
	# if {$xsb_trace} {puts "xsb_query received: $line"}
    }
    return [xsb_get_relation $line relname $numresult]
}
#
# declares a tcl variable as also a Prolog variable. Whenever the tcl
# variable is updated, the Prolog variable will be updated accordingly.
# Whenever the Prolog variable is updated by the Prolog program using
# tcl_set/2, the corresponding tcl variable will be updated as well.
#
proc xsb_var args {
	global xsb_trace
	foreach var $args {
		global $var
		trace vdelete $var wu xsb_update_var
		trace variable $var wu xsb_update_var
		if {$xsb_trace} {puts "trace put on $var"}
	}
}
#
# to send all changed variables to Prolog
proc xsb_tcl_send {} {
	global xsb_set_list
	if {$xsb_set_list != {}} {
		set cmd {tcl_set_vars([}
		set var [lindex $xsb_set_list 0]
		if {[regexp {^(.*)\((.*)\)$} $var all vara element]} {
			global $vara
			set val [eval set val "\$$vara\($element\)"]
		} else {
			global $var
			set val [eval set val \$$var]
		}
		append cmd $var,'$val'
		foreach var [lrange $xsb_set_list 1 end] {
			if {[regexp {^(.*)\((.*)\)$} $var all vara element]} {
				global $vara
				set val [eval set val "\$$vara\($element\)"]
			} else {
				global $var
				set val [eval set val \$$var]
			}
			append cmd ,$var,'$val'
		}
		append cmd {])}
		set xsb_set_list ""
		xsb_command0 $cmd
	}
}

#
# Used in trace to catch tcl var updates and propagate them
# (Here op will always be "w", since that's how trace is set.)
#
proc xsb_update_var {var element op} {
	global xsb_set_list xsb_trace xsb_tflag
	if {$xsb_tflag} {return}
	if {$element != ""} {set var ${var}($element)}
	if {$op == "u"} {set var "unset($var)"}
	if {[lsearch $xsb_set_list "$var"] < 0} {
		lappend xsb_set_list "$var"
	}
}
#
proc xsb_tcl_set {args} {
	global xsb_trace xsb_tflag 
	set xsb_tflag 1
	for {set i 0} {$i < [llength $args]} {incr i 3} {
		set var [lindex $args $i]
		set element [lindex $args [expr $i+1]]
		set val [lindex $args [expr $i+2]]
		global $var
		if {$element != ""} {set var ${var}($element)}
		set $var $val
	}
	set xsb_tflag 0
}
#
#
#
proc xsb_unset {args} {
	set xsb_tflag 1
	foreach var $args {
		global $var
		catch {unset $var}
	}
	set xsb_tflag 0
}
#
# Creates a TK frame containing a relation as an array of label widgets.
# The name of the frame to create is the first argument. The second
# argument is the name of a global (array) variable containing the relation
# (as created by xsb_query)
# [needs more work. must make scrollable, and support column headings.]
# [also crashes if given an empty relation.]
#
proc xsb_relation_d {rframe relname} {
	upvar $relname rn
	set i 1
	set nfields $rn(nfields)
	set ntuples $rn(ntuples)
	while {$i <= $nfields} {
		set colname ${rframe}.col$i
		frame $colname
		set j 0
		while {$j <= $ntuples} {
			set cellname ${rframe}.col$i.row$j
			label $cellname -relief ridge -text $rn($j,$i)
			if {$j == 0} {$cellname configure -relief flat}
			lappend cells $cellname
			incr j 
		}
		eval "pack $cells -side top -anchor w -fill x"
		lappend columns $colname
		incr i
	}
	eval "pack $columns -side left"
}
#
# Creates a TK frame containing a relation as an array of entry widgets.
# The name of the frame to create is the first argument. The second
# argument is the name of a global (array) variable containing the relation
# (as created by xsb_query)
# [needs more work. again scrollable, and column names. perhaps incremental
# update back in Prolog? maybe not.]
# 
proc xsb_relation_u {rframe relname} {
	upvar $relname rn
	set nfields $rn(nfields)
	set ntuples $rn(ntuples)
	set i 1
	while {$i <= $nfields} {
		set colname ${rframe}.col$i
		frame $colname
		set j 0
		while {$j <= $ntuples} {
			set cellname ${rframe}.col$i.row$j
			entry $cellname -relief ridge \
				-textvariable ${relname}($j,$i) \
				-width [string length ${relname}($j,$i)]
			if {$j == 0} {$cellname configure -relief flat}
			lappend cells $cellname
			incr j 
		}
		eval pack $cells -side top -anchor w -fill x
		lappend columns $colname
		incr i
	}
	eval pack $columns -side left
}
#
#
#
proc xsb_prolog_toploop {} {
	global xsb_proc xsb_trace
	xsb_tcl_send
	set line "| ?- "
	while "1" {
		puts -nonewline $line
		set anotherquery 1
		while {$anotherquery} {
			gets stdin query
			if {($query == "halt.") | ($query == "pause.") | \
						($query == "quit.")} {
				return 1
			}
			puts $xsb_proc $query ; flush $xsb_proc
			if {[string range $query [expr [string length $query] \
							- 1] end] == "."} {
				gets $xsb_proc line
				if {$line == "*** syntax error ***"} {
				    while {$line != ""} {
					puts $line
					gets $xsb_proc line
				    }
				    puts $line
				} else {set anotherquery 0}
			}
		}
		set linefeed 0
		while {$line != "| ?- "} {
			if {$line == ":"} {
				gets stdin line
				puts $xsb_proc $line ; flush $xsb_proc
				gets $xsb_proc line
				set linefeed 0
			} elseif {[string range $line 0 4] == "|tcl-"} {
				if {$xsb_trace} {
					puts "xsb_query evaling: $line"}
				eval [string range $line 5 end]
				gets $xsb_proc line
			} else {
				if {$linefeed} {puts ""}
				puts -nonewline $line
				set linefeed 1
				gets $xsb_proc line
			}
		}
		if {$linefeed} {puts ""}
	}

}
