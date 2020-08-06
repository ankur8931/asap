# Copyright (c) 1999, Bryan Oakley
# All Rights Reservered
#
# Bryan Oakley
# oakley@channelpoint.com
#
# mclistbox v1.02 March 30, 1999
#
# a multicolumn listbox written in pure tcl
#
# this code is freely distributable without restriction, but is 
# provided as-is with no waranty expressed or implied. 
#
# basic usage: 
#    
#    mclistbox::mclistbox .listbox
#    .listbox column add col1 -label "Column 1"
#    .listbox column add col2 -label "Column 2"
#    .listbox insert end [list "some stuff" "some more stuff"]
#    .listbox insert end [list "a second row of stuff" "blah blah blah"]
#
# see the documentation for more, uh, documentation.
#
# Something to think about: implement a "-optimize" option, with two 
# values: speed and memory. If set to speed, keep a copy of the data
# in our hidden listbox so retrieval of data doesn't require us to
# do all the getting and splitting and so forth. If set to "memory",
# bag saving a duplicate copy of the data, which means data retrieval
# will be slower, but memory requirements will be reduced. 

package require Tk 8.0
package provide mclistbox 1.02

namespace eval ::mclistbox {

    # this is the public interface
    namespace export mclistbox

    # these contain references to available options
    variable widgetOptions
    variable columnOptions

    # these contain references to available commands and subcommands
    variable widgetCommands
    variable columnCommands
    variable labelCommands
}

# ::mclistbox::Init --
#
#     Initialize the global (well, namespace) variables. This should
#     only be called once, immediately prior to creating the first
#     instance of the widget
#
# Results:
#
#     All state variables are set to their default values; all of 
#     the option database entries will exist.
#
# Returns:
# 
#     empty string

proc ::mclistbox::Init {} {
    variable widgetOptions
    variable columnOptions
    variable widgetCommands
    variable columnCommands
    variable labelCommands

    # here we match up command line options with option database names
    # and classes. As it turns out, this is a handy reference of all of the
    # available options. Note that if an item has a value with only one
    # item (like -bd, for example) it is a synonym and the value is the
    # actual item.

    array set widgetOptions [list \
	    -background          {background          Background} \
	    -bd                  -borderwidth \
	    -bg                  -background \
	    -borderwidth         {borderWidth         BorderWidth} \
	    -columnbd            -columnborderwidth \
	    -columnborderwidth   {columnBorderWidth   BorderWidth} \
	    -columnrelief        {columnRelief        Relief} \
	    -cursor              {cursor              Cursor} \
	    -exportselection     {exportSelection     ExportSelection} \
	    -fg                  -foreground \
	    -fillcolumn          {fillColumn          FillColumn} \
	    -font                {font                Font} \
	    -foreground          {foreground          Foreground} \
	    -height              {height              Height} \
	    -highlightbackground {highlightBackground HighlightBackground} \
	    -highlightcolor      {highlightColor      HighlightColor} \
	    -highlightthickness  {highlightThickness  HighlightThickness} \
	    -labelanchor         {labelAnchor         Anchor} \
	    -labelbackground     {labelBackground     Background} \
	    -labelbd             -labelborderwidth \
	    -labelbg             -labelbackground \
	    -labelborderwidth    {labelBorderWidth    BorderWidth} \
	    -labelfg             -labelforeground \
	    -labelfont           {labelFont           Font} \
	    -labelforeground     {labelForeground     Foreground} \
	    -labelheight         {labelHeight         Height} \
	    -labelimage          {labelImage          Image} \
	    -labelrelief         {labelRelief         Relief} \
	    -labels              {labels              Labels} \
	    -relief              {relief              Relief} \
	    -resizablecolumns    {resizableColumns    ResizableColumns} \
	    -selectbackground    {selectBackground    Foreground} \
	    -selectborderwidth   {selectBorderWidth   BorderWidth} \
	    -selectcommand       {selectCommand       Command} \
	    -selectforeground    {selectForeground    Background} \
	    -selectmode          {selectMode          SelectMode} \
	    -setgrid             {setGrid             SetGrid} \
	    -takefocus           {takeFocus           TakeFocus} \
	    -width               {width               Width} \
	    -xscrollcommand      {xScrollCommand      ScrollCommand} \
	    -yscrollcommand      {yScrollCommand      ScrollCommand} \
	    ]

    # and likewise for column-specific stuff. 
    array set columnOptions [list \
	    -background         {background           Background} \
	    -bitmap		{bitmap               Bitmap} \
	    -font               {font                 Font} \
	    -foreground         {foreground           Foreground} \
	    -image              {image                Image} \
	    -label 		{label                Label} \
	    -position           {position             Position} \
	    -resizable          {resizable            Resizable} \
	    -visible            {visible              Visible} \
	    -width              {width                Width} \
            ]

    # this defines the valid widget commands. It's important to
    # list them here; we use this list to validate commands and
    # expand abbreviations.
    set widgetCommands [list \
	    activate	 bbox       cget     column    configure  \
	    curselection delete     get      index     insert \
	    label        nearest    scan     see       selection  \
	    size         xview      yview
    ]

    set columnCommands [list add cget configure delete names nearest]
    set labelCommands  [list bind]

    ######################################################################
    #- this initializes the option database. Kinda gross, but it works
    #- (I think). 
    ######################################################################

    set packages [package names]

    # why check for the Tk package? This lets us be sourced into 
    # an interpreter that doesn't have Tk loaded, such as the slave
    # interpreter used by pkg_mkIndex. In theory it should have no
    # side effects when run 
    if {[lsearch -exact [package names] "Tk"] != -1} {

	# compute a widget name we can use to create a temporary widget
	set tmpWidget ".__tmp__"
	set count 0
	while {[winfo exists $tmpWidget] == 1} {
	    set tmpWidget ".__tmp__$count"
	    incr count
	}

	# steal options from the listbox
	# we want darn near all options, so we'll go ahead and do
	# them all. No harm done in adding the one or two that we
	# don't use.
	listbox $tmpWidget 
	foreach foo [$tmpWidget configure] {
	    if {[llength $foo] == 5} {
		set option [lindex $foo 1]
		set value [lindex $foo 4]
		option add *Mclistbox.$option $value widgetDefault

		# these options also apply to the individual columns...
		if {[string compare $option "foreground"] == 0 \
			|| [string compare $option "background"] == 0 \
			|| [string compare $option "font"] == 0} {
		    option add *Mclistbox*MclistboxColumn.$option $value \
			    widgetDefault
		}
	    }
	}
	destroy $tmpWidget

	# steal some options from label widgets; we only want a subset
	# so we'll use a slightly different method. No harm in *not*
	# adding in the one or two that we don't use... :-)
	label $tmpWidget
	foreach option [list Anchor Background Font \
		Foreground Height Image  ] {
	    set values [$tmpWidget configure -[string tolower $option]]
	    option add *Mclistbox.label$option [lindex $values 3]
	}
	destroy $tmpWidget

	# these are unique to us...
	option add *Mclistbox.columnBorderWidth   0      widgetDefault
	option add *Mclistbox.columnRelief        flat   widgetDefault
	option add *Mclistbox.labelBorderWidth    1      widgetDefault
	option add *Mclistbox.labelRelief         raised widgetDefault
	option add *Mclistbox.labels              1      widgetDefault
	option add *Mclistbox.resizableColumns    1      widgetDefault
	option add *Mclistbox.selectcommand       {}     widgetDefault
	option add *Mclistbox.fillcolumn          {}     widgetDefault

	# column options
	option add *Mclistbox*MclistboxColumn.visible       1      widgetDefault
	option add *Mclistbox*MclistboxColumn.resizable     1      widgetDefault
	option add *Mclistbox*MclistboxColumn.position      end    widgetDefault
	option add *Mclistbox*MclistboxColumn.label         ""     widgetDefault
	option add *Mclistbox*MclistboxColumn.width         0      widgetDefault
	option add *Mclistbox*MclistboxColumn.bitmap        ""     widgetDefault
	option add *Mclistbox*MclistboxColumn.image         ""     widgetDefault
    }

    ######################################################################
    # define the class bindings
    ######################################################################
    
    SetClassBindings

}

# ::mclistbox::mclistbox --
#
#     This is the command that gets exported. It creates a new
#     mclistbox widget.
#
# Arguments:
#
#     w        path of new widget to create
#     args     additional option/value pairs (eg: -background white, etc.)
#
# Results:
#
#     It creates the widget and sets up all of the default bindings
#
# Returns:
#
#     The name of the newly create widget

proc ::mclistbox::mclistbox {args} {
    variable widgetOptions

    # perform a one time initialization
    if {![info exists widgetOptions]} {
	Init
    }

    # make sure we at least have a widget name
    if {[llength $args] == 0} {
	error "wrong # args: should be \"mclistbox pathName ?options?\""
    }

    # ... and make sure a widget doesn't already exist by that name
    if {[winfo exists [lindex $args 0]]} {
	error "window name \"[lindex $args 0]\" already exists"
    }

    # and check that all of the args are valid
    foreach {name value} [lrange $args 1 end] {
	Canonize [lindex $args 0] option $name
    }

    # build it...
    set w [eval Build $args]

    # set some bindings...
    SetBindings $w

    # and we are done!
    return $w
}

# ::mclistbox::Build --
#
#    This does all of the work necessary to create the basic
#    mclistbox. 
#
# Arguments:
#
#    w        widget name
#    args     additional option/value pairs
#
# Results:
#
#    Creates a new widget with the given name. Also creates a new
#    namespace patterened after the widget name, as a child namespace
#    to ::mclistbox
#
# Returns:
#
#    the name of the widget

proc ::mclistbox::Build {w args} {
    variable widgetOptions

    # create the namespace for this instance, and define a few
    # variables
    namespace eval ::mclistbox::$w {

	variable options
	variable widgets
	variable misc 
    }

    # this gives us access to the namespace variables within
    # this proc
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    # initially we start out with no columns
    set misc(columns) {}

    # this is our widget -- a frame of class Mclistbox. Naturally,
    # it will contain other widgets. We create it here because
    # we need it to be able to set our default options.
    set widgets(this)   [frame  $w -class Mclistbox -takefocus 1]

    # this defines all of the default options. We get the
    # values from the option database. Note that if an array
    # value is a list of length one it is an alias to another
    # option, so we just ignore it
    foreach name [array names widgetOptions] {
	if {[llength $widgetOptions($name)] == 1} continue
	set optName  [lindex $widgetOptions($name) 0]
	set optClass [lindex $widgetOptions($name) 1]
	set options($name) [option get $w $optName $optClass]
    }

    # now apply any of the options supplied on the command
    # line. This may overwrite our defaults, which is OK
    if {[llength $args] > 0} {
	array set options $args
    }
    
    # the columns all go into a text widget since it has the 
    # ability to scroll.
    set widgets(text) [text $w.text \
	    -width 0 \
	    -height 0 \
	    -padx 0 \
	    -pady 0 \
	    -wrap none \
	    -borderwidth 0 \
	    -highlightthickness 0 \
	    -takefocus 0 \
	    -cursor {} \
	    ]

    $widgets(text) configure -state disabled

    # here's the tricky part (shhhh... don't tell anybody!)
    # we are going to create column that completely fills 
    # the base frame. We will use it to control the sizing
    # of the widget. The trick is, we'll pack it in the frame 
    # and then place the text widget over it so it is never
    # seen.

    set columnWidgets [NewColumn $w {__hidden__}]
    set widgets(hiddenFrame)   [lindex $columnWidgets 0]
    set widgets(hiddenListbox) [lindex $columnWidgets 1]
    set widgets(hiddenLabel)   [lindex $columnWidgets 2]

    # by default geometry propagation is turned off, but for this
    # super-secret widget we want it turned on. The idea is, we 
    # resize the listbox which resizes the frame which resizes the 
    # whole shibang.
    pack propagate $widgets(hiddenFrame) on

    pack $widgets(hiddenFrame) -side top -fill both -expand y
    place $widgets(text) -x 0 -y 0 -relwidth 1.0 -relheight 1.0
    raise $widgets(text)

    # we will later rename the frame's widget proc to be our
    # own custom widget proc. We need to keep track of this
    # new name, so we'll define and store it here...
    set widgets(frame) ::mclistbox::${w}::$w

    # this moves the original frame widget proc into our
    # namespace and gives it a handy name
    rename ::$w $widgets(frame)

    # now, create our widget proc. Obviously (?) it goes in
    # the global namespace. All mclistbox widgets will actually
    # share the same widget proc to cut down on the amount of
    # bloat. 
    proc ::$w {command args} \
	    "eval ::mclistbox::WidgetProc {$w} \$command \$args"

    # ok, the thing exists... let's do a bit more configuration. 
    if {[catch "Configure $widgets(this) [array get options]" error]} {
	catch {destroy $w}
    }

    # and be prepared to handle selections.. (this, for -exportselection
    # support)
    selection handle $w [list ::mclistbox::SelectionHandler $w get]

    return $w
}

# ::mclistbox::SelectionHandler --
#
#    handle reqests to set or retrieve the primary selection. This is
#    the "guts" of the implementation of the -exportselection option.
#    What a pain! Note that this command is *not* called as a result
#    of the widget's "selection" command, but rather as a result of
#    the global selection being set or cleared.
#
#    If I read the ICCCM correctly (which is doubtful; who has time to
#    read that thing thoroughly?), this should return each row as a tab
#    separated list of values, and the whole as a newline separated
#    list of rows.
#
# Arguments:
#
#    w       pathname of the widget
#    type    one of "own", "lose" or "get"
#    offset  only used if type is "get"; offset into the selection
#            buffer where the returned data should begin
#    length  number of bytes to return
#

proc ::mclistbox::SelectionHandler {w type {offset ""} {length ""}} {
    upvar ::mclistbox::${w}::options   options
    upvar ::mclistbox::${w}::misc      misc
    upvar ::mclistbox::${w}::widgets   widgets

    switch -exact $type {

	own {
	    selection own \
		    -command [list ::mclistbox::SelectionHandler $w lose] \
		    -selection PRIMARY \
		    $w
	}

	lose {
	    if {$options(-exportselection)} {
		foreach id $misc(columns) {
		    $widgets(listbox$id) selection clear 0 end
		}
	    }
	}

	get {
	    set end [expr {$length + $offset - 1}]

	    set column [lindex $misc(columns) 0]
	    set curselection [$widgets(listbox$column) curselection]

	    # this is really, really slow (relatively speaking).
	    # but the only way I can think of to speed this up
	    # is to duplicate all the data in our hidden listbox,
	    # which I really don't want to do because of memory
	    # considerations.
	    set data ""
	    foreach index $curselection {
		set rowdata [join [::mclistbox::WidgetProc-get $w $index]  "\t"]
		lappend data $rowdata
	    }
	    set data [join $data "\n"]
	    return [string range $data $offset $end]
	}

    }
}

# ::mclistbox::convert --
#
#     public routine to convert %x, %y and %W binding substitutions.
#     Given an x, y and or %W value relative to a given widget, this
#     routine will convert the values to be relative to the mclistbox
#     widget. For example, it could be used in a binding like this:
#
#     bind .mclistbox <blah> {doSomething [::mclistbox::convert %W -x %x]}
#
#     Note that this procedure is *not* exported, but is indented for
#     public use. It is not exported because the name could easily 
#     clash with existing commands. 
#
# Arguments:
#
#     w     a widget path; typically the actual result of a %W 
#           substitution in a binding. It should be either a
#           mclistbox widget or one of its subwidgets
#
#     args  should one or more of the following arguments or 
#           pairs of arguments:
#
#           -x <x>      will convert the value <x>; typically <x> will
#                       be the result of a %x substitution
#           -y <y>      will convert the value <y>; typically <y> will
#                       be the result of a %y substitution
#           -W (or -w)  will return the name of the mclistbox widget
#                       which is the parent of $w
#
# Returns:
#
#     a list of the requested values. For example, a single -w will
#     result in a list of one items, the name of the mclistbox widget.
#     Supplying "-x 10 -y 20 -W" (in any order) will return a list of
#     three values: the converted x and y values, and the name of 
#     the mclistbox widget.

proc ::mclistbox::convert {w args} {
    set result {}
    if {![winfo exists $w]} {
	error "window \"$w\" doesn't exist"
    }

    while {[llength $args] > 0} {
	set option [lindex $args 0]
	set args [lrange $args 1 end]

	switch -exact -- $option {
	    -x {
		set value [lindex $args 0]
		set args [lrange $args 1 end]
		set win $w
		while {[winfo class $win] != "Mclistbox"} {
		    incr value [winfo x $win]
		    set win [winfo parent $win]
		    if {$win == "."} break
		}
		lappend result $value
	    }

	    -y {
		set value [lindex $args 0]
		set args [lrange $args 1 end]
		set win $w
		while {[winfo class $win] != "Mclistbox"} {
		    incr value [winfo y $win]
		    set win [winfo parent $win]
		    if {$win == "."} break
		}
		lappend result $value
	    }

	    -w -
	    -W {
		set win $w
		while {[winfo class $win] != "Mclistbox"} {
		    set win [winfo parent $win]
		    if {$win == "."} break;
		}
		lappend result $win
	    }
	}
    }
    return $result
}

# ::mclistbox::SetBindings --
#
#    Sets up the default bindings for the named widget
#
# Arguments:
#
#    w   the widget pathname for which the bindings should be assigned
#
# Results:
#
#    The named widget will inheirit all of the default Mclistbox
#    bindings.

proc ::mclistbox::SetBindings {w} {
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    # we must do this so that the columns fill the text widget in
    # the y direction
    bind $widgets(text) <Configure> \
	    [list ::mclistbox::AdjustColumns $w %h]

}

# ::mclistbox::SetClassBindings --
#
#    Sets up the default bindings for the widget class
#
# Arguments:
#
#    none
#

proc ::mclistbox::SetClassBindings {} {
    # this allows us to clean up some things when we go away
    bind Mclistbox <Destroy> [list ::mclistbox::DestroyHandler %W]

    # steal all of the standard listbox bindings. Note that if a user
    # clicks in a column, %W will return that column. This is bad,
    # so we have to make a substitution in all of the bindings to
    # compute the real widget name (ie: the name of the topmost 
    # frame)
    foreach event [bind Listbox] {
	set binding [bind Listbox $event]
	regsub -all {%W} $binding {[::mclistbox::convert %W -W]} binding
	regsub -all {%x} $binding {[::mclistbox::convert %W -x %x]} binding
	regsub -all {%y} $binding {[::mclistbox::convert %W -y %y]} binding
	bind Mclistbox $event $binding
    }

    # these define bindings for the column labels for resizing. Note
    # that we need both the name of this widget (calculated by $this)
    # as well as the specific widget that the event occured over.
    # Also note that $this is a constant string that gets evaluated
    # when the binding fires.
    # What a pain.
    set this {[::mclistbox::convert %W -W]}
    bind MclistboxMouseBindings <ButtonPress-1> \
	    "::mclistbox::ResizeEvent $this buttonpress %W %x %X %Y"
    bind MclistboxMouseBindings <ButtonRelease-1> \
	    "::mclistbox::ResizeEvent $this buttonrelease %W %x %X %Y"
    bind MclistboxMouseBindings <Enter> \
	    "::mclistbox::ResizeEvent $this motion %W %x %X %Y"
    bind MclistboxMouseBindings <Motion> \
	    "::mclistbox::ResizeEvent $this motion %W %x %X %Y"
    bind MclistboxMouseBindings <B1-Motion> \
	    "::mclistbox::ResizeEvent $this drag %W %x %X %Y"
}

# ::mclistbox::NewColumn --
#
#    Adds a new column to the mclistbox widget
#
# Arguments:
#
#    w     the widget pathname
#    id    the id for the new column
#
# Results:
#
#    Creates a set of widgets which defines the column. Adds 
#    appropriate entries to the global array widgets for the
#    new column. 
#
#    Note that this column is not added to the listbox by 
#    this proc.
#
# Returns:
#
#    A list of three elements: the path to the column frame, 
#    the path to the column listbox, and the path to the column
#    label, in that order.

proc ::mclistbox::NewColumn {w id} {
    upvar ::mclistbox::${w}::widgets   widgets
    upvar ::mclistbox::${w}::options   options
    upvar ::mclistbox::${w}::misc      misc
    upvar ::mclistbox::${w}::columnID  columnID

    # the columns are all children of the text widget we created... 
    set frame     \
	    [frame $w.frame$id \
	    -takefocus 0 \
	    -highlightthickness 0 \
	    -class MclistboxColumn \
	    -background $options(-background) \
	    ]

    set listbox   \
	    [listbox $frame.listbox \
	    -takefocus 0 \
	    -bd 0 \
	    -setgrid $options(-setgrid) \
	    -exportselection false \
	    -selectmode $options(-selectmode) \
	    -highlightthickness 0 \
	    ]

    set label     \
	    [label $frame.label \
	    -takefocus 0 \
	    -relief raised \
	    -bd 1 \
	    -highlightthickness 0 \
	    ]

    # define mappings from widgets to columns
    set columnID($label) $id
    set columnID($frame) $id
    set columnID($listbox) $id

    # we're going to associate a new bindtag for the label to
    # handle our resize bindings. Why? We want the bindings to
    # be specific to this widget but we don't want to use the
    # widget name. If we use the widget name then the bindings
    # could get mixed up with user-supplied bindigs (via the 
    # "label bind" command). 
    set tag MclistboxLabel
    bindtags $label  [list MclistboxMouseBindings $label]

    # reconfigure the label based on global options
    foreach option [list bd image height relief font anchor \
	    background foreground borderwidth] {
	if {[info exists options(-label$option)] \
		&& $options(-label$option) != ""} {
	    $label configure -$option $options(-label$option)
	}
    }

    # reconfigure the column based on global options
    foreach option [list borderwidth relief] {
	if {[info exists options(-column$option)] \
		&& $options(-column$option) != ""} {
	    $frame configure -$option $options(-column$option)
	}
    }

    # geometry propagation must be off so we can control the size
    # of the listbox by setting the size of the containing frame
    pack propagate $frame off

    pack $label   -side top -fill x -expand n
    pack $listbox -side top -fill both -expand y -pady 2

    # any events that happen in the listbox gets handled by the class
    # bindings. This has the unfortunate side effect 
    bindtags $listbox [list $w Mclistbox all]

    # return a list of the widgets we created.
    return [list $frame $listbox $label]
}

# ::mclistbox::Column-add --
#
#    Implements the "column add" widget command
#
# Arguments:
#
#    w      the widget pathname
#    args   additional option/value pairs which define the column
#
# Results:
#
#    A column gets created and added to the listbox

proc ::mclistbox::Column-add {w args} {
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    variable widgetOptions

    set id "column-[llength $misc(columns)]" ;# a suitable default

    # if the first argument doesn't have a "-" as the first
    # character, it is an id to associate with this column
    if {![string match {-*} [lindex $args 0]]} {
	# the first arg must be an id. 
	set id [lindex $args 0]
	set args [lrange $args 1 end]
	if {[lsearch -exact $misc(columns) $id] != -1} {
	    error "column \"$id\" already exists"
	}
    }

    # define some reasonable defaults, then add any specific
    # values supplied by the user
    set opts(-bitmap)     {}
    set opts(-image)      {}
    set opts(-visible)    1
    set opts(-resizable)  1
    set opts(-position)   "end"
    set opts(-width)      20
    set opts(-background) $options(-background)
    set opts(-foreground) $options(-foreground)
    set opts(-font)       $options(-font)
    set opts(-label)      $id

    if {[expr {[llength $args]%2}] == 1} {
	# hmmm. An odd number of elements in args
	# if the last item is a valid option we'll give a different
	# error than if its not
	set option [::mclistbox::Canonize $w "column option" [lindex $args end]]
	error "value for \"[lindex $args end]\" missing"
    }
    array set opts $args

    # figure out if we have any data in the listbox yet; we'll need
    # this information in a minute...
    if {[llength $misc(columns)] > 0} {
	set col0 [lindex $misc(columns) 0]
	set existingRows [$widgets(listbox$col0) size]
    } else {
	set existingRows 0
    }

    # create the widget and assign the associated paths to our array
    set widgetlist [NewColumn $w $id]

    set widgets(frame$id)   [lindex $widgetlist 0]
    set widgets(listbox$id) [lindex $widgetlist 1]
    set widgets(label$id)   [lindex $widgetlist 2]
    
    # add this column to the list of known columns
    lappend misc(columns) $id

    # configure the options. As a side effect, it will be inserted
    # in the text widget
    eval ::mclistbox::Column-configure {$w} {$id} [array get opts]

    # now, if there is any data already in the listbox, we need to
    # add a corresponding number of blank items. At least, I *think*
    # that's the right thing to do.
    if {$existingRows > 0} {
	set blanks {}
	for {set i 0} {$i < $existingRows} {incr i} {
	    lappend blanks {}
	}
	eval {$widgets(listbox$id)} insert end $blanks
    }

    InvalidateScrollbars $w
    return $id
}

# ::mclistbox::Column-configure --
#
#    Implements the "column configure" widget command
#
# Arguments:
#
#    w     widget pathname
#    id    column identifier
#    args  list of option/value pairs

proc ::mclistbox::Column-configure {w id args} {
    variable widgetOptions
    variable columnOptions

    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    # bail if they gave us a bogus id
    set index [CheckColumnID $w $id]

    # define some shorthand
    set listbox $widgets(listbox$id)
    set frame   $widgets(frame$id)
    set label   $widgets(label$id)

    if {[llength $args] == 0} {
	# hmmm. User must be wanting all configuration information
	# note that if the value of an array element is of length
	# one it is an alias, which needs to be handled slightly
	# differently
	set results {}
	foreach opt [lsort [array names columnOptions]] {
	    if {[llength $columnOptions($opt)] == 1} {
		set alias $columnOptions($opt)
		set optName $columnOptions($alias)
		lappend results [list $opt $optName]
	    } else {
		set optName  [lindex $columnOptions($opt) 0]
		set optClass [lindex $columnOptions($opt) 1]
		set default [option get $frame $optName $optClass]
		lappend results [list $opt $optName $optClass \
			$default $options($id:$opt)]
	    }
	}

	return $results


    } elseif {[llength $args] == 1} {

	# the user must be querying something... I need to get this
	# to return a bona fide list like the "real" configure 
	# command, but it's not a priority at the moment. I still
	# have to work on the option database support foo.
	set option [::mclistbox::Canonize $w "column option" [lindex $args 0]]

	set value $options($id:$option)
	set optName  [lindex $columnOptions($option) 0]
	set optClass [lindex $columnOptions($option) 1]
	set default  [option get $frame $optName $optClass]
	set results  [list $option $optName $optClass $default $value]
	return $results

    }

    # if we have an odd number of values, bail. 
    if {[expr {[llength $args]%2}] == 1} {
	# hmmm. An odd number of elements in args
	error "value for \"[lindex $args end]\" missing"
    }
    
    # Great. An even number of options. Let's make sure they 
    # are all valid before we do anything. Note that Canonize
    # will generate an error if it finds a bogus option; otherwise
    # it returns the canonical option name
    foreach {name value} $args {
	set name [::mclistbox::Canonize $w "column option" $name]
	set opts($name) $value
    }

    # if we get to here, the user is wanting to set some options
    foreach option [array names opts] {
	set value $opts($option)
	set options($id:$option) $value

	switch -- $option {
	    -label {
		$label configure -text $value
	    }
	    
	    -image -
	    -bitmap {
		$label configure $option $value
	    }

	    -width {
		set font [$listbox cget -font]
		set factor [font measure $options(-font) "0"]
		set width [expr {$value * $factor}]

		$widgets(frame$id) configure -width $width
		set misc(min-$widgets(frame$id)) $width
		AdjustColumns $w
	    }
	    -font -
	    -foreground -
	    -background {
		if {[string length $value] == 0} {set value $options($option)}
		$listbox configure $option $value
	    }

	    -resizable {
		if {[catch {
		    if {$value} {
			set options($id:-resizable) 1
		    } else {
			set options($id:-resizable) 0
		    }
		} msg]} {
		    error "expected boolean but got \"$value\""
		}
	    }

	    -visible {
		if {[catch {
		    if {$value} {
			set options($id:-visible) 1
			$widgets(text) configure -state normal
			$widgets(text) window configure 1.$index -window $frame
			$widgets(text) configure -state disabled

		    } else {
			set options($id:-visible) 0
			$widgets(text) configure -state normal
			$widgets(text) window configure 1.$index -window {}
			$widgets(text) configure -state disabled
		    }
		    InvalidateScrollbars $w
		} msg]} {
		    error "expected boolean but got \"$value\""
		}

	    }

	    -position {
		if {[string compare $value "start"] == 0} {
		    set position 0

		} elseif {[string compare $value "end"] == 0} {

		    set position [expr {[llength $misc(columns)] -1}]
		} else {

		    # ought to check for a legal value here, but I'm 
		    # lazy
		    set position $value
		}

		if {$position >= [llength $misc(columns)]} {
		    set max [expr {[llength $misc(columns)] -1}]
		    error "bad position; must be in the range of 0-$max"
		}

		# rearrange misc(columns) to reflect the new ordering
		set current [lsearch -exact $misc(columns) $id]
		set misc(columns) [lreplace $misc(columns) $current $current]
		set misc(columns) [linsert $misc(columns) $position $id]
		
		set frame $widgets(frame$id)
		$widgets(text) configure -state normal
		$widgets(text) window create 1.$position \
			-window $frame -stretch 1
		$widgets(text) configure -state disabled
 	    }

	}
    }
}


# ::mclistbox::DestroyHandler {w} --
# 
#    Cleans up after a mclistbox widget is destroyed
#
# Arguments:
#
#    w    widget pathname
#
# Results:
#
#    The namespace that was created for the widget is deleted,
#    and the widget proc is removed.

proc ::mclistbox::DestroyHandler {w} {

    # kill off any idle event we might have pending
    if {[info exists ::mclistbox::${w}::misc(afterid)]} {
	catch {
	    after cancel $::mclistbox::${w}::misc(afterid)
	    unset ::mclistbox::${w}::misc(afterid)
	}
    }

    # if the widget actually being destroyed is of class Mclistbox,
    # crush the namespace and kill the proc. Get it? Crush. Kill. 
    # Destroy. Heh. Danger Will Robinson! Oh, man! I'm so funny it
    # brings tears to my eyes.
    if {[string compare [winfo class $w] "Mclistbox"] == 0} {
	namespace delete ::mclistbox::$w
	rename $w {}
    }

}

# ::mclistbox::MassageIndex --
#
#    this proc massages indicies of the form @x,y such that 
#    the coordinates are relative to the first listbox rather 
#    than relative to the topmost frame. 
#
# Arguments:
#
#    w       widget pathname
#    index   an index of the form @x,y
#
# Results:
#
#    Returns a new index with translated coordinates. This index
#    may be used directly by an internal listbox.

proc ::mclistbox::MassageIndex {w index} {
    upvar ::mclistbox::${w}::widgets   widgets
    upvar ::mclistbox::${w}::misc      misc

    if {[regexp {@([0-9]+),([0-9]+)} $index matchvar x y]} {
	set id [lindex $misc(columns) 0]
	
	incr y -[winfo y $widgets(listbox$id)]
	incr y -[winfo y $widgets(frame$id)]
	incr x [winfo x $widgets(listbox$id)]
	incr x [winfo x $widgets(frame$id)]

	set index @${x},${y}
    }

    return $index
}

# ::mclistbox::WidgetProc --
#
#    This gets uses as the widgetproc for an mclistbox widget. 
#    Notice where the widget is created and you'll see that the
#    actual widget proc merely evals this proc with all of the
#    arguments intact.
#
#    Note that some widget commands are defined "inline" (ie:
#    within this proc), and some do most of their work in 
#    separate procs. This is merely because sometimes it was
#    easier to do it one way or the other.
#
# Arguments:
#
#    w         widget pathname
#    command   widget subcommand
#    args      additional arguments; varies with the subcommand
#
# Results:
#
#    Performs the requested widget command

proc ::mclistbox::WidgetProc {w command args} {
    variable widgetOptions

    upvar ::mclistbox::${w}::widgets   widgets
    upvar ::mclistbox::${w}::options   options
    upvar ::mclistbox::${w}::misc      misc
    upvar ::mclistbox::${w}::columnID  columnID

    set command [::mclistbox::Canonize $w command $command]

    # some commands have subcommands. We'll check for that here 
    # and mung the command and args so that we can treat them as 
    # distinct commands in the following switch statement
    if {[string compare $command "column"] == 0} {
	set subcommand [::mclistbox::Canonize $w "column command" \
		[lindex $args 0]]
	set command "$command-$subcommand"
	set args [lrange $args 1 end]

    } elseif {[string compare $command "label"] == 0} {
	set subcommand [::mclistbox::Canonize $w "label command" \
		[lindex $args 0]]
	set command "$command-$subcommand"
	set args [lrange $args 1 end]
    }

    set result ""
    catch {unset priorSelection}

    # here we go. Error checking be damned!
    switch $command {
	xview {
	    # note that at present, "xview <index>" is broken. I'm
	    # not even sure how to do it. Unless I attach our hidden
	    # listbox to the scrollbar and use it. Hmmm..... I'll
	    # try that later. (FIXME)
	    set result [eval {$widgets(text)} xview $args]
	    InvalidateScrollbars $w
	}

	yview {
	    if {[llength $args] == 0} {
		# length of zero means to fetch the yview; we can
		# get this from a single listbox
		set result [$widgets(hiddenListbox) yview]

	    } else {

		# if it's one argument, it's an index. We'll pass that 
		# index through the index command to properly translate
		# @x,y indicies, and place the value back in args
		if {[llength $args] == 1} {
		    set index [::mclistbox::MassageIndex $w [lindex $args 0]]
		    set args [list $index]
		}

		# run the yview command on every column.
		foreach id $misc(columns) {
		    eval {$widgets(listbox$id)} yview $args
		}
		eval {$widgets(hiddenListbox)} yview $args

		InvalidateScrollbars $w

		set result ""
	    }
	}

	activate {
	    if {[llength $args] != 1} {
		error "wrong \# of args: should be $w activate index"
	    }
	    set index [::mclistbox::MassageIndex $w [lindex $args 0]]

	    foreach id $misc(columns) {
		$widgets(listbox$id) activate $index
	    }
	    set result ""
	}

	bbox {
	    if {[llength $args] != 1} {
		error "wrong \# of args: should be $w bbox index"
	    }
	    # get a real index. This will adjust @x,y indicies
	    # to account for the label, if any.
	    set index [::mclistbox::MassageIndex $w [lindex $args 0]]

	    set id [lindex $misc(columns) 0]

	    # we can get the x, y, and height from the first 
	    # column.
	    set bbox [$widgets(listbox$id) bbox $index]
	    if {[string length $bbox] == 0} {return ""}

	    foreach {x y w h} $bbox {}
	    
	    # the x and y coordinates have to be adjusted for the
	    # fact that the listbox is inside a frame, and the 
	    # frame is inside a text widget. All of those add tiny
	    # offsets. Feh.
	    incr y [winfo y $widgets(listbox$id)]
	    incr y [winfo y $widgets(frame$id)]
	    incr x [winfo x $widgets(listbox$id)]
	    incr x [winfo x $widgets(frame$id)]

	    # we can get the width by looking at the relative x 
	    # coordinate of the right edge of the last column
	    set id [lindex $misc(columns) end]
	    set w [expr {[winfo width $widgets(frame$id)] + \
		    [winfo x $widgets(frame$id)]}]
	    set bbox [list $x $y [expr {$x + $w}] $h]
	    set result $bbox
	}

	label-bind {
	    # we are just too clever for our own good. (that's a 
	    # polite way of saying this is more complex than it
	    # needs to be)

	    set id [lindex $args 0]
	    set index [CheckColumnID $w $id]

	    set args [lrange $args 1 end]
	    if {[llength $args] == 0} {
		set result [bind $widgets(label$id)]
	    } else {

		# when we create a binding, we'll actually have the 
		# binding run our own command with the user's command
		# as an argument. This way we can do some sanity checks
		# before running the command. So, when querying a binding
		# we need to only return the user's code
		set sequence [lindex $args 0]
		if {[llength $args] == 1} {
		    set result [lindex [bind $widgets(label$id) $sequence] end]
		} else {
		
		    # replace %W with our toplevel frame, then
		    # do the binding
		    set code [lindex $args 1]
		    regsub -all {%W} $code $w code
		    
		    set result [bind $widgets(label$id) $sequence \
			    [list ::mclistbox::LabelEvent $w $id $code]]
		}
	    }
	}

	column-add {
	    eval ::mclistbox::Column-add {$w} $args
	    AdjustColumns $w
	    set result ""
	}

	column-delete {
	    foreach id $args {
		set index [CheckColumnID $w $id]

		# remove the reference from our list of columns
		set misc(columns) [lreplace $misc(columns) $index $index]

		# whack the widget
		destroy $widgets(frame$id)

		# clear out references to the individual widgets
		unset widgets(frame$id)
		unset widgets(listbox$id)
		unset widgets(label$id)
	    }
	    InvalidateScrollbars $w
	    set result ""
	}

	column-cget {
	    if {[llength $args] != 2} {
		error "wrong # of args: should be \"$w column cget name option\""
	    }
	    set id [::mclistbox::Canonize $w column [lindex $args 0]]
	    set option [lindex $args 1]
	    set data [::mclistbox::Column-configure $w $id $option]
	    set result [lindex $data 4]
	}

	column-configure {
	    set id [::mclistbox::Canonize $w column [lindex $args 0]]
	    set args [lrange $args 1 end]
	    set result [eval ::mclistbox::Column-configure {$w} {$id} $args]
	}

	column-names {
	    if {[llength $args] != 0} {
		error "wrong # of args: should be \"$w column names\""
	    }
	    set result $misc(columns)
	}

	column-nearest {
	    if {[llength $args] != 1} {
		error "wrong # of args: should be \"$w column nearest x\""
	    }

	    set x [lindex $args 0]
	    set tmp [$widgets(text) index @$x,0]
	    set tmp [split $tmp "."]
	    set index [lindex $tmp 1]

	    set result [lindex $misc(columns) $index]
	}

	cget {
	    if {[llength $args] != 1} {
		error "wrong # args: should be $w cget option"
	    }
	    set opt [::mclistbox::Canonize $w option [lindex $args 0]]

	    set result $options($opt)
	}


	configure {
	    set result [eval ::mclistbox::Configure {$w} $args]

	}

	curselection {
	    set id [lindex $misc(columns) 0]
	    set result [$widgets(listbox$id) curselection]
	}

	delete {
	    if {[llength $args] < 1 || [llength $args] > 2} {
		error "wrong \# of args: should be $w delete first ?last?"
	    }

	    # it's possible that the selection will change because
	    # of something we do. So, grab the current selection before
	    # we do anything. Just before returning we'll see if the
	    # selection has changed. If so, we'll call our selectcommand
	    if {$options(-selectcommand) != ""} {
		set col0 [lindex $misc(columns) 0]
		set priorSelection [$widgets(listbox$col0) curselection]
	    }

	    set index1 [::mclistbox::MassageIndex $w [lindex $args 0]]
	    if {[llength $args] == 2} {
		set index2 [::mclistbox::MassageIndex $w [lindex $args 1]]
	    } else {
		set index2 ""
	    }

	    # note we do an eval here to make index2 "disappear" if it
	    # is set to an empty string.
	    foreach id $misc(columns) {
		eval {$widgets(listbox$id)} delete $index1 $index2
	    }
	    eval {$widgets(hiddenListbox)} delete $index1 $index2

	    InvalidateScrollbars $w

	    set result ""
	}

	get {
	    if {[llength $args] < 1 || [llength $args] > 2} {
		error "wrong \# of args: should be $w get first ?last?"
	    }
	    set index1 [::mclistbox::MassageIndex $w [lindex $args 0]]
	    if {[llength $args] == 2} {
		set index2 [::mclistbox::MassageIndex $w [lindex $args 1]]
	    } else {
		set index2 ""
	    }

	    set result [eval ::mclistbox::WidgetProc-get {$w} $index1 $index2]

	}

	index {

	    if {[llength $args] != 1} {
		error "wrong \# of args: should be $w index index"
	    }

	    set index [::mclistbox::MassageIndex $w [lindex $args 0]]
	    set id [lindex $misc(columns) 0]

	    set result [$widgets(listbox$id) index $index]
	}

	insert {
	    if {[llength $args] < 1} {
		error "wrong \# of args: should be $w insert ?element \
		      element...?"
	    }

	    # it's possible that the selection will change because
	    # of something we do. So, grab the current selection before
	    # we do anything. Just before returning we'll see if the
	    # selection has changed. If so, we'll call our selectcommand
	    if {$options(-selectcommand) != ""} {
		set col0 [lindex $misc(columns) 0]
		set priorSelection [$widgets(listbox$col0) curselection]
	    }

	    set index [::mclistbox::MassageIndex $w [lindex $args 0]]

	    ::mclistbox::Insert $w $index [lrange $args 1 end]

	    InvalidateScrollbars $w
	    set result ""
	}

	nearest {
	    if {[llength $args] != 1} {
		error "wrong \# of args: should be $w nearest y"
	    }

	    # translate the y coordinate into listbox space
	    set id [lindex $misc(columns) 0]
	    set y [lindex $args 0]
	    incr y -[winfo y $widgets(listbox$id)]
	    incr y -[winfo y $widgets(frame$id)]

	    set col0 [lindex $misc(columns) 0]

	    set result [$widgets(listbox$col0) nearest $y]
	}

	scan {
	    foreach {subcommand x y} $args {}
	    switch $subcommand {
		mark {
		    # we have to treat scrolling in x and y differently;
		    # scrolling in the y direction affects listboxes and
		    # scrolling in the x direction affects the text widget.
		    # to facilitate that, we need to keep a local copy
		    # of the scan mark.
		    set misc(scanmarkx) $x
		    set misc(scanmarky) $y
		    
		    # set the scan mark for each column
		    foreach id $misc(columns) {
			$widgets(listbox$id) scan mark $x $y
		    }

		    # we can't use the x coordinate given us, since it 
		    # is relative to whatever column we are over. So,
		    # we'll just usr the results of [winfo pointerx].
		    $widgets(text) scan mark [winfo pointerx $w]  $y
		}
		dragto {
		    # we want the columns to only scan in the y direction,
		    # so we'll force the x componant to remain constant
		    foreach id $misc(columns) {
			$widgets(listbox$id) scan dragto $misc(scanmarkx) $y
		    }

		    # since the scan mark of the text widget was based
		    # on the pointer location, so must be the x
		    # coordinate to the dragto command. And since we
		    # want the text widget to only scan in the x
		    # direction, the y componant will remain constant
		    $widgets(text) scan dragto \
			    [winfo pointerx $w] $misc(scanmarky)

		    # make sure the scrollbars reflect the changes.
		    InvalidateScrollbars $w
		}

		set result ""
	    }
	}

	see {
	    if {[llength $args] != 1} {
		error "wrong \# of args: should be $w see index"
	    }
	    set index [::mclistbox::MassageIndex $w [lindex $args 0]]

	    foreach id $misc(columns) {
		$widgets(listbox$id) see $index
	    }
	    InvalidateScrollbars $w
	    set result {}
	}

	selection {
	    # it's possible that the selection will change because
	    # of something we do. So, grab the current selection before
	    # we do anything. Just before returning we'll see if the
	    # selection has changed. If so, we'll call our selectcommand
	    if {$options(-selectcommand) != ""} {
		set col0 [lindex $misc(columns) 0]
		set priorSelection [$widgets(listbox$col0) curselection]
	    }

	    set subcommand [lindex $args 0]
	    set args [lrange $args 1 end]

	    set prefix "wrong \# of args: should be $w"
	    switch $subcommand {
		includes {
		    if {[llength $args] != 1} {
			error "$prefix selection $subcommand index"
		    }
		    set index [::mclistbox::MassageIndex $w [lindex $args 0]]
		    set id [lindex $misc(columns) 0]
		    set result [$widgets(listbox$id) selection includes $index]
		}

		set {
		    switch [llength $args] {
			1 {
			    set index1 [::mclistbox::MassageIndex $w \
				    [lindex $args 0]]
			    set index2 ""
			}
			2 {
			    set index1 [::mclistbox::MassageIndex $w \
				    [lindex $args 0]]
			    set index2 [::mclistbox::MassageIndex $w \
				    [lindex $args 1]]
			}
			default {
			    error "$prefix selection clear first ?last?"
			}
		    }

		    if {$options(-exportselection)} {
			SelectionHandler $w own
		    }
		    if {$index1 != ""} {
			foreach id $misc(columns) {
			    eval {$widgets(listbox$id)} selection set \
				    $index1 $index2
			}
		    }

		    set result ""
		}

		anchor {
		    if {[llength $args] != 1} {
			error "$prefix selection $subcommand index"
		    }
		    set index [::mclistbox::MassageIndex $w [lindex $args 0]]

		    if {$options(-exportselection)} {
			SelectionHandler $w own
		    }
		    foreach id $misc(columns) {
			$widgets(listbox$id) selection anchor $index
		    }
		    set result ""
		}

		clear {
		    switch [llength $args] {
			1 {
			    set index1 [::mclistbox::MassageIndex $w \
				    [lindex $args 0]]
			    set index2 ""
			}
			2 {
			    set index1 [::mclistbox::MassageIndex $w \
				    [lindex $args 0]]
			    set index2 [::mclistbox::MassageIndex $w \
				    [lindex $args 1]]
			}
			default {
			    error "$prefix selection clear first ?last?"
			}
		    }

		    if {$options(-exportselection)} {
			SelectionHandler $w own
		    }
		    foreach id $misc(columns) {
			eval {$widgets(listbox$id)} selection clear \
				$index1 $index2
		    }
		    set result ""
		}
	    }
	}

	size {
	    set id [lindex $misc(columns) 0]
	    set result [$widgets(listbox$id) size]
	}
    }

    # if the user has a selectcommand defined and the selection changed,
    # run the selectcommand
    if {[info exists priorSelection] && $options(-selectcommand) != ""} {
	set column [lindex $misc(columns) 0]
	set currentSelection [$widgets(listbox$column) curselection]
	if {[string compare $priorSelection $currentSelection] != 0} {
	    # this logic keeps us from getting into some sort of
	    # infinite loop of the selectcommand changes the selection
	    # (not particularly well tested, but it seems like the
	    # right thing to do...)
	    if {![info exists misc(skipRecursiveCall)]} {
		set misc(skipRecursiveCall) 1
		uplevel \#0 $options(-selectcommand) $currentSelection
		catch {unset misc(skipRecursiveCall)}
	    }
	}
    }

    return $result
}

# ::mclistbox::WidgetProc-get --
#
#    Implements the "get" widget command
#
# Arguments:
#
#    w      widget path
#    args   additional arguments to the get command

proc ::mclistbox::WidgetProc-get {w args} {
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    set returnType "list"
    # the listbox "get" command returns different things
    # depending on whether it has one or two args. Internally
    # we *always* want a valid list, so we'll force a second
    # arg which in turn forces the listbox to return a list,
    # even if its a list of one element
    if {[llength $args] == 1} {
	lappend args [lindex $args 0]
	set returnType "listOfLists"
    }

    # get all the data from each column
    foreach id $misc(columns) {
	set data($id) [eval {$widgets(listbox$id)} get $args]
    }

    # now join the data together one row at a time. Ugh.
    set result {}
    set rows [llength $data($id)]
    for {set i 0} {$i < $rows} {incr i} {
	set this {}
	foreach column $misc(columns) {
	    lappend this [lindex $data($column) $i]
	}
	lappend result $this
    }
    
    # now to unroll the list if necessary. If the user gave
    # us only one indicie we want to return a single list
    # of values. If they gave use two indicies we want to return
    # a list of lists.
    if {[string compare $returnType "list"] == 0} {
	return $result
    } else {
	return [lindex $result 0]
    }
}

# ::mclistbox::CheckColumnID --
#
#    returns the index of the id within our list of columns, or 
#    reports an error if the id is invalid
#
# Arguments:
#
#    w    widget pathname
#    id   a column id
#
# Results:
#
#   Will compute and return the index of the column within the
#   list of columns (which happens to be it's -position, as it
#   turns out) or returns an error if the named column doesn't
#   exist.

proc ::mclistbox::CheckColumnID {w id} {
    upvar ::mclistbox::${w}::misc    misc

    set id [::mclistbox::Canonize $w column $id]
    set index [lsearch -exact $misc(columns) $id]
    return $index
}

# ::mclistbox::LabelEvent --
#
#    Handle user events on the column labels for the Mclistbox
#    class. 
#
# Arguments:
#
#    w        widget pathname
#    id       a column identifier
#    code     tcl code to be evaluated.
#
# Results:
#
#    Executes the code associate with an event, but only if the
#    event wouldn't otherwise potentially trigger a resize event.
#
#    We use the cursor of the label to let us know whether the
#    code should be executed. If it is set to the cursor of the
#    mclistbox widget, the code will be executed. It is assumed
#    that if it is not the same cursor, it is the resize cursor
#    which should only be set if the cursor is very near a border
#    of a label and the column is resizable.

proc ::mclistbox::LabelEvent {w id code} {
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options

    # only fire the binding if the cursor is our default cursor
    # (ie: if we aren't in a "resize zone")
    set cursor [$widgets(label$id) cget -cursor]
    if {[string compare $cursor $options(-cursor)] == 0} {
	uplevel \#0 $code
    }
}

# ::mclistbox::HumanizeList --
#
#    Returns a human-readable form of a list by separating items
#    by columns, but separating the last two elements with "or"
#    (eg: foo, bar or baz)
#
# Arguments:
#
#    list    a valid tcl list
#
# Results:
#
#    A string which as all of the elements joined with ", " or 
#    the word " or "

proc ::mclistbox::HumanizeList {list} {

    if {[llength $list] == 1} {
	return [lindex $list 0]
    } else {
	set list [lsort $list]
	set secondToLast [expr {[llength $list] -2}]
	set most [lrange $list 0 $secondToLast]
	set last [lindex $list end]

	return "[join $most {, }] or $last"
    }
}

# ::mclistbox::Canonize --
#
#    takes a (possibly abbreviated) option or command name and either 
#    returns the canonical name or an error
#
# Arguments:
#
#    w        widget pathname
#    object   type of object to canonize; must be one of "command",
#             "option", "column" or "column option".
#    opt      the option (or command) to be canonized
#
# Returns:
#
#    Returns either the canonical form of an option or command,
#    or raises an error if the option or command is unknown or
#    ambiguous.

proc ::mclistbox::Canonize {w object opt} {
    variable widgetOptions
    variable columnOptions
    variable widgetCommands
    variable columnCommands
    variable labelCommands

    switch $object {
	command {
	    if {[lsearch -exact $widgetCommands $opt] >= 0} {
		return $opt
	    }

	    # command names aren't stored in an array, and there
	    # isn't a way to get all the matches in a list, so
	    # we'll stuff the columns in a temporary array so
	    # we can use [array names]
	    set list $widgetCommands
	    foreach element $list {
		set tmp($element) ""
	    }
	    set matches [array names tmp ${opt}*]
	}

	{label command} {
	    if {[lsearch -exact $labelCommands $opt] >= 0} {
		return $opt
	    }

	    # command names aren't stored in an array, and there
	    # isn't a way to get all the matches in a list, so
	    # we'll stuff the columns in a temporary array so
	    # we can use [array names]
	    set list $labelCommands
	    foreach element $list {
		set tmp($element) ""
	    }
	    set matches [array names tmp ${opt}*]
	}

	{column command} {
	    if {[lsearch -exact $columnCommands $opt] >= 0} {
		return $opt
	    }

	    # command names aren't stored in an array, and there
	    # isn't a way to get all the matches in a list, so
	    # we'll stuff the columns in a temporary array so
	    # we can use [array names]
	    set list $columnCommands
	    foreach element $list {
		set tmp($element) ""
	    }
	    set matches [array names tmp ${opt}*]
	}

	option {
	    if {[info exists widgetOptions($opt)] \
		    && [llength $widgetOptions($opt)] == 3} {
		return $opt
	    }
	    set list [array names widgetOptions]
	    set matches [array names widgetOptions ${opt}*]
	}

	{column option} {
	    if {[info exists columnOptions($opt)]} {
		return $opt
	    }
	    set list [array names columnOptions]
	    set matches [array names columnOptions ${opt}*]
	}

	column {
	    upvar ::mclistbox::${w}::misc    misc

	    if {[lsearch -exact $misc(columns) $opt] != -1} {
		return $opt
	    }
	    
	    # column names aren't stored in an array, and there
	    # isn't a way to get all the matches in a list, so
	    # we'll stuff the columns in a temporary array so
	    # we can use [array names]
	    set list $misc(columns)
	    foreach element $misc(columns) {
		set tmp($element) ""
	    }
	    set matches [array names tmp ${opt}*]
	}
    }
    if {[llength $matches] == 0} {
	set choices [HumanizeList $list]
	error "unknown $object \"$opt\"; must be one of $choices"

    } elseif {[llength $matches] == 1} {
	# deal with option aliases
	set opt [lindex $matches 0]
	switch $object {
	    option {
		if {[llength $widgetOptions($opt)] == 1} {
		    set opt $widgetOptions($opt)
		}
	    }

	    {column option} {
		if {[llength $columnOptions($opt)] == 1} {
		    set opt $columnOptions($opt)
		}
	    }
	}

	return $opt

    } else {
	set choices [HumanizeList $list]
	error "ambiguous $object \"$opt\"; must be one of $choices"
    }
}

# ::mclistbox::Configure --
#
#    Implements the "configure" widget subcommand
#
# Arguments:
#
#    w      widget pathname
#    args   zero or more option/value pairs (or a single option)
#
# Results:
#    
#    Performs typcial "configure" type requests on the widget
 
proc ::mclistbox::Configure {w args} {
    variable widgetOptions

    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc
    
    if {[llength $args] == 0} {
	# hmmm. User must be wanting all configuration information
	# note that if the value of an array element is of length
	# one it is an alias, which needs to be handled slightly
	# differently
	set results {}
	foreach opt [lsort [array names widgetOptions]] {
	    if {[llength $widgetOptions($opt)] == 1} {
		set alias $widgetOptions($opt)
		set optName $widgetOptions($alias)
		lappend results [list $opt $optName]
	    } else {
		set optName  [lindex $widgetOptions($opt) 0]
		set optClass [lindex $widgetOptions($opt) 1]
		set default [option get $w $optName $optClass]
		lappend results [list $opt $optName $optClass \
			$default $options($opt)]
	    }
	}

	return $results
    }
    
    # one argument means we are looking for configuration
    # information on a single option
    if {[llength $args] == 1} {
	set opt [::mclistbox::Canonize $w option [lindex $args 0]]

	set optName  [lindex $widgetOptions($opt) 0]
	set optClass [lindex $widgetOptions($opt) 1]
	set default [option get $w $optName $optClass]
	set results [list $opt $optName $optClass \
		$default $options($opt)]
	return $results
    }

    # if we have an odd number of values, bail. 
    if {[expr {[llength $args]%2}] == 1} {
	# hmmm. An odd number of elements in args
	error "value for \"[lindex $args end]\" missing"
    }
    
    # Great. An even number of options. Let's make sure they 
    # are all valid before we do anything. Note that Canonize
    # will generate an error if it finds a bogus option; otherwise
    # it returns the canonical option name
    foreach {name value} $args {
	set name [::mclistbox::Canonize $w option $name]
	set opts($name) $value
    }

    # process all of the configuration options
    foreach option [array names opts] {

	set newValue $opts($option)
	if {[info exists options($option)]} {
	    set oldValue $options($option)
#	    set options($option) $newValue
	}

	switch -- $option {
	    -exportselection {
		if {$newValue} {
		    SelectionHandler $w own
		    set options($option) 1
		} else {
		    set options($option) 0
		}
	    }

	    -fillcolumn {
		# if the fill column changed, we need to adjust
		# the columns.
		AdjustColumns $w
		set options($option) $newValue
	    }

	    -takefocus {
		$widgets(frame) configure -takefocus $newValue
		set options($option) [$widgets(frame) cget $option]
	    }

	    -background {
		foreach id $misc(columns) {
		    $widgets(listbox$id) configure -background $newValue
		    $widgets(frame$id) configure   -background $newValue
		}
		$widgets(frame) configure -background $newValue

		$widgets(text) configure -background $newValue
		set options($option) [$widgets(frame) cget $option]
	    }

	    # { the following all must be applied to each listbox }
	    -foreground -
	    -font -
	    -selectborderwidth -
	    -selectforeground -
	    -selectbackground -
	    -setgrid {
		foreach id $misc(columns) {
		    $widgets(listbox$id) configure $option $newValue
		}
		$widgets(hiddenListbox) configure $option $newValue
		set options($option) [$widgets(hiddenListbox) cget $option]
	    }

	    # { the following all must be applied to each listbox and frame }
	    -cursor {
		foreach id $misc(columns) {
		    $widgets(listbox$id) configure $option $newValue
		    $widgets(frame$id) configure -cursor $newValue
		}

		# -cursor also needs to be applied to the 
		# frames of each column
		foreach id $misc(columns) {
		    $widgets(frame$id) configure -cursor $newValue
		}

		$widgets(hiddenListbox) configure $option $newValue
		set options($option) [$widgets(hiddenListbox) cget $option]
	    }

	    # { this just requires to pack or unpack the labels }
	    -labels {
		if {$newValue} {
		    set newValue 1
		    foreach id $misc(columns) {
			pack $widgets(label$id) \
				-side top -fill x -expand n \
				-before $widgets(listbox$id)
		    }
		    pack $widgets(hiddenLabel) \
			    -side top -fill x -expand n \
			    -before $widgets(hiddenListbox)

		} else {
		    set newValue 
		    foreach id $misc(columns) {
			pack forget $widgets(label$id)
		    }
		    pack forget $widgets(hiddenLabel)
		}
		set options($option) $newValue
	    }

	    -height {
		$widgets(hiddenListbox) configure -height $newValue
		InvalidateScrollbars $w
		set options($option) [$widgets(hiddenListbox) cget $option]
	    }

	    -width {
		if {$newValue == 0} {
		    error "a -width of zero is not supported. "
		}

		$widgets(hiddenListbox) configure -width $newValue
		InvalidateScrollbars $w
		set options($option) [$widgets(hiddenListbox) cget $option]
	    }

	    # { the following all must be applied to each column frame }
	    -columnborderwidth -
	    -columnrelief {
		regsub {column} $option {} listboxoption
		foreach id $misc(columns) {
		    $widgets(listbox$id) configure $listboxoption $newValue
		}
		$widgets(hiddenListbox) configure $listboxoption $newValue
		set options($option) [$widgets(hiddenListbox) cget \
			$listboxoption]
	    }

	    -resizablecolumns {
		if {$newValue} {
		    set options($option) 1
		} else {
		    set options($option) 0
		}
	    }
	    
	    # { the following all must be applied to each column header }
	    -labelimage -
	    -labelheight -
	    -labelrelief -
	    -labelfont -
	    -labelanchor -
	    -labelbackground -
	    -labelforeground -
	    -labelborderwidth {
		regsub {label} $option {} labeloption
		foreach id $misc(columns) {
		    $widgets(label$id) configure $labeloption $newValue
		}
		$widgets(hiddenLabel) configure $labeloption $newValue
		set options($option) [$widgets(hiddenLabel) cget $labeloption]
	    }

	    # { the following apply only to the topmost frame}
	    -borderwidth -
	    -highlightthickness -
	    -highlightcolor -
	    -highlightbackground -
	    -relief {
		$widgets(frame) configure $option $newValue
		set options($option) [$widgets(frame) cget $option]
	    }

	    -selectmode {
		set options($option) $newValue
	    }

	    -selectcommand {
		set options($option) $newValue
	    }

	    -xscrollcommand {
		InvalidateScrollbars $w
		set options($option) $newValue
	    }

	    -yscrollcommand {
		InvalidateScrollbars $w
		set options($option) $newValue
	    }
	}
    }
}

# ::mclistbox::UpdateScrollbars --
#
#    This proc does the work of actually update the scrollbars to
#    reflect the current view
#
# Arguments:
#
#    w      widget pathname
#
# Results:
#
#    Potentially changes the size or placement of the scrollbars
#    (if any) based on changes to the widget

proc ::mclistbox::UpdateScrollbars {w} {
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    if {![winfo ismapped $w]} {
	catch {unset misc(afterid)}
	return
    }

    update idletasks
    if {[llength $misc(columns)] > 0} {
	if {[string length $options(-yscrollcommand)] != 0} {
	    set col0 [lindex $misc(columns) 0]
	    set yview [$widgets(listbox$col0) yview]
	    eval $options(-yscrollcommand) $yview
	}

	if {[string length $options(-xscrollcommand)] != 0} {
	    set col0 [lindex $misc(columns) 0]
	    set xview [$widgets(text) xview]
	    eval $options(-xscrollcommand) $xview
	}
    }
    catch {unset misc(afterid)}
}

# ::mclistbox::InvalidateScrollbars --
#
#    Schedules the scrollbars to be updated the next time 
#    we are idle.
#
# Arguments:
#
#    w     widget pathname
#
# Results:
#
#    sets up a proc to be run in the idle event handler

proc ::mclistbox::InvalidateScrollbars {w} {

    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    if {![info exists misc(afterid)]} {
	set misc(afterid) \
		[after idle "catch {::mclistbox::UpdateScrollbars $w}"]
    }
}

# ::mclistbox::Insert --
#
#    This implements the "insert" widget command; it arranges for 
#    one or more elements to be inserted into the listbox.
#
# Arguments:
#
#    w         widget pathname
#    index     a valid listbox index to designate where the data is
#              to be inserted
#    arglist   A list of values to be inserted. Each element of the
#              list will itself be treated as a list, one element for
#              each column.
#
# Results:
#
#    Inserts the data into the list and updates the scrollbars

proc ::mclistbox::Insert {w index arglist} {

    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    foreach list $arglist {
	# make sure we have enough elements for each column
	for {set i [llength $list]} {$i < [llength $misc(columns)]} {incr i} {
	    lappend list {}
	}

	set column 0
	foreach id $misc(columns) {
	    $widgets(listbox$id) insert $index [lindex $list $column]
	    incr column
	}

	# we also want to add a bogus item to the hidden listbox. Why?
	# For standard listboxes, if you specify a height of zero the
	# listbox will resize to be just big enough to hold all the lines.
	# Since we use a hidden listbox to regulate the size of the widget
	# and we want this same behavior, this listbox needs the same number
	# of elements as the visible listboxes
	#
	# (NB: we might want to make this listbox contain the contents
	# of all columns as a properly formatted list; then the get 
	# command can query this listbox instead of having to query
	# each individual listbox. The disadvantage is that it doubles
	# the memory required to hold all the data)
	$widgets(hiddenListbox) insert $index "x"
    }
    return ""
}

# ::mclistbox::ColumnIsHidden --
#
#    Returns a boolean representing whether a column is visible or
#    not
#
# Arguments:
#
#    w     widget pathname
#    id    a column identifier
#
# Results:
#
#    returns 1 if the column is visible (ie: not hidden), or 0
#    if invisible. Note that the result doesn't consider whether
#    the column is actually viewable. Even if it has been scrolled
#    off screen, 1 will be returned as long as the column hasn't
#    been hidden by turning the visibility off.

proc ::mclistbox::ColumnIsHidden {w id} {
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::misc    misc
    
    set retval 1
    set col [lsearch -exact $misc(columns) $id]

    if {$col != ""} {
	set index "1.$col"
	catch {
	    set window [$widgets(text) window cget $index -window]
	    if {[string length $window] > 0 && [winfo exists $window]} {
		set retval 0
	    }
	}
    }
    return $retval
}

# ::mclistbox::AdjustColumns --
#
#    Adjusts the height and width of the individual columns.
#
# Arguments:
#
#    w       widget pathname
#    height  height, in pixels, that the columns should be adjusted
#            to. If null, the height will be the height of the text
#            widget that underlies our columns.
#
# Results:
#
#    All columns will be adjusted to fill the text widget in the y
#    direction. Also, if a -fillcolumn is defined, that column will
#    be grown, if necessary, to fill the widget in the x direction.

proc ::mclistbox::AdjustColumns {w {height ""}} {
    upvar ::mclistbox::${w}::widgets widgets
    upvar ::mclistbox::${w}::options options
    upvar ::mclistbox::${w}::misc    misc

    if {[string length $height] == 0} {
	set height [winfo height $widgets(text)]
    }

    # resize the height of each column so it matches the height
    # of the text widget, minus a few pixels. 
    incr height -4
    foreach id $misc(columns) {
	$widgets(frame$id) configure -height $height
    }
    
    # if we have a fillcolumn, change its width accordingly
    if {$options(-fillcolumn) != ""} {

	# make sure the column has been defined. If not, bail (?)
	if {![info exists widgets(frame$options(-fillcolumn))]} {
	    return
	}
	set frame $widgets(frame$options(-fillcolumn))
	set minwidth $misc(min-$frame)

	# compute current width of all columns
	set colwidth 0
	set col 0
	foreach id $misc(columns) {
	    if {![ColumnIsHidden $w $id] && $id != $options(-fillcolumn)} {
		incr colwidth [winfo reqwidth $widgets(frame$id)]
	    }
	}

	# this is just shorthand for later use...
	set id $options(-fillcolumn)

	# compute optimal width
	set optwidth [expr {[winfo width $widgets(text)] - \
		(2 * [$widgets(text) cget -padx])}]

	# compute the width of our fill column
	set newwidth [expr {$optwidth - $colwidth}]

	if {$newwidth < $minwidth} {
	    set newwidth $minwidth
	}

	# adjust the width of our fill column frame
	$widgets(frame$id) configure -width $newwidth
	    
    }
    InvalidateScrollbars $w
}

# ::mclistbox::FindResizableNeighbor --
#
#    Returns the nearest resizable column to the left or right
#    of the named column. 
#
# Arguments:
#
#    w          widget pathname
#    id         column identifier
#    direction  should be one of "right" or "left". Actually, anything
#               that doesn't match "right" will be treated as "left"
#
# Results:
#
#    Will return the column identifier of the nearest resizable
#    column, or an empty string if none exists. 

proc ::mclistbox::FindResizableNeighbor {w id {direction right}} {
    upvar ::mclistbox::${w}::widgets       widgets
    upvar ::mclistbox::${w}::options       options
    upvar ::mclistbox::${w}::misc          misc


    if {$direction == "right"} {
	set incr 1
	set stop [llength $misc(columns)]
	set start [expr {[lsearch -exact $misc(columns) $id] + 1}]
    } else {
	set incr -1
	set stop -1
	set start [expr {[lsearch -exact $misc(columns) $id] - 1}]
    } 

    for {set i $start} {$i != $stop} {incr i $incr} {
	set col [lindex $misc(columns) $i]
	if {![ColumnIsHidden $w $col] && $options($col:-resizable)} {
	    return $col
	}
    }
    return ""
}

# ::mclistbox::ResizeEvent --
#
#    Handles the events which implement interactive column resizing
#    using the mouse.
#
# Arguments:
#
#    w        widget pathname
#    type     type of event; must be one of "buttonpress", "drag",
#             "motion", or "buttonrelease"
#    widget   the actual widget that the event occured over
#    x        the x coordinate of the mouse, relative to $widget
#    X        the root x coordinate of the mouse
#    Y        the root y coordinate of the mouse
#
# The basic idea is this:
#
# whenever the cursor moves over the label, we examine it's x
# coordinate to determine if its within a fixed amount of
# pixels from the left or right edge. If it is, we reconfigure
# the cursor to be a suitable "this thing is resizable" cursor.
#
# On a buttonclick, if the cursor is not the default cursor 
# (and thus, presumably the resize cursor), we set up some
# state for an eventual resize. We figure out which columns
# are to the left and right and base a maximum resize amount
# for each direction. We also define the absolute X coordinate
# of the buttonpress as a reference point for the drag.
#
# on a b1-motion, if the drag state exists, we look at the 
# absolute X value and use it to compute a delta value from
# the reference (the X of the button press). We then resize the
# left and right column frames by the delta amount.
#
# on a button release, we unset the state and the cursor, which
# cancels all of the above.

proc ::mclistbox::ResizeEvent {w type widget x X Y} {

    upvar ::mclistbox::${w}::widgets       widgets
    upvar ::mclistbox::${w}::options       options
    upvar ::mclistbox::${w}::misc          misc
    upvar ::mclistbox::${w}::columnID      columnID

    # if the widget doesn't allow resizable cursors, there's
    # nothing here to do...
    if {!$options(-resizablecolumns)} {
	return
    }

    # this lets us keep track of some internal state while
    # the user is dragging the mouse
    variable drag

    # this lets us define a small window around the edges of
    # the column. 
    set threshold [expr {$options(-labelborderwidth) + 4}]

    # this is what we use for the "this is resizable" cursor
    set resizeCursor sb_h_double_arrow

    # if we aren't over an area that we care about, bail.
    if {![info exists columnID($widget)]} {
	return
    }

    # id refers to the column name
    set id $columnID($widget)

    switch $type {

	buttonpress {
	    # we will do all the work of initiating a drag only if
	    # the cursor is not the defined cursor. In theory this
	    # will only be the case if the mouse moves over the area
	    # in which a drag can happen.
	    if {[$widgets(label$id) cget -cursor] == $resizeCursor} {
		if {$x <= $threshold} {
		    set lid [::mclistbox::FindResizableNeighbor $w $id left]
		    if {$lid == ""} return
		    set drag(leftFrame)  $widgets(frame$lid)
		    set drag(rightFrame) $widgets(frame$id)

		    set drag(leftListbox)  $widgets(listbox$lid)
		    set drag(rightListbox) $widgets(listbox$id)

		} else {
		    set rid [::mclistbox::FindResizableNeighbor $w $id right]
		    if {$rid == ""} return
		    set drag(leftFrame)  $widgets(frame$id)
		    set drag(rightFrame) $widgets(frame$rid)

		    set drag(leftListbox)  $widgets(listbox$id)
		    set drag(rightListbox) $widgets(listbox$rid)

		}
		

		set drag(leftWidth)  [winfo width $drag(leftFrame)]
		set drag(rightWidth) [winfo width $drag(rightFrame)]

		# it seems to be a fact that windows can never be 
		# less than one pixel wide. So subtract that one pixel
		# from our max deltas...
		set drag(maxDelta)   [expr {$drag(rightWidth) - 1}]
		set drag(minDelta)  -[expr {$drag(leftWidth) - 1}]

		set drag(x) $X
	    }
	}

	motion {
	    if {[info exists drag(x)]} {return}

	    # this is just waaaaay too much work for a motion 
	    # event, IMO.

	    set resizable 0

	    # is the column the user is over resizable?
	    if {!$options($id:-resizable)} {return}

	    # did the user click on the left of a column? 
	    if {$x < $threshold} {
		set leftColumn [::mclistbox::FindResizableNeighbor $w $id left]
		if {$leftColumn != ""} {
		    set resizable 1
		}

	    } elseif {$x > [winfo width $widget] - $threshold} {
		set rightColumn [::mclistbox::FindResizableNeighbor $w $id \
			right]
		if {$rightColumn != ""} {
		    set resizable 1
		}
	    }

	    # if it's resizable, change the cursor 
	    set cursor [$widgets(label$id) cget -cursor]
	    if {$resizable && $cursor != $resizeCursor} {
		$widgets(label$id) configure -cursor $resizeCursor

	    } elseif {!$resizable && $cursor == $resizeCursor} {
		$widgets(label$id) configure -cursor $options(-cursor)
	    }
	}

	drag {
	    # if the state is set up, do the drag...
	    if {[info exists drag(x)]} {

		set delta [expr {$X - $drag(x)}]
		if {$delta >= $drag(maxDelta)} {
		    set delta $drag(maxDelta)

		} elseif {$delta <= $drag(minDelta)} {
		    set delta $drag(minDelta)
		}

		set lwidth [expr {$drag(leftWidth) + $delta}]
		set rwidth [expr {$drag(rightWidth) - $delta}]

		$drag(leftFrame)   configure -width $lwidth
		$drag(rightFrame)  configure -width $rwidth

	    }
	}

	buttonrelease {
	    set fillColumnID $options(-fillcolumn)
	    if {[info exists drag(x)] && $fillColumnID != {}} {
		set fillColumnFrame $widgets(frame$fillColumnID)
		if {[string compare $drag(leftFrame) $fillColumnFrame] == 0 \
			|| [string compare $drag(rightFrame) $fillColumnFrame] == 0} {
		    set width [$fillColumnFrame cget -width]
		    set misc(minFillColumnSize) $width
		}
		set misc(min-$drag(leftFrame))  [$drag(leftFrame) cget -width]
		set misc(min-$drag(rightFrame)) [$drag(rightFrame) cget -width]
	    }

	    # reset the state and the cursor
	    catch {unset drag}
	    $widgets(label$id) configure -cursor $options(-cursor)

	}
    }
}

# end of mclistbox.tcl
