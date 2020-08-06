Usage:	xsb [flags] [-l] 
	xsb [flags] module
	xsb [flags] -B boot_module [-D cmd_loop_driver] [-t]
	xsb [flags] -B module_to_disassemble -d
	xsb [-h | --help | -v | --version]

	module:
	     Module to execute after XSB starts up.
	     Module should have no suffixes, no directory part, and
	     the file module.xwam must be on the library search path.
      	boot_module:
             This is a developer's option.
	     The -B flags tells XSB which bootstrapping module to use instead
	     of the standard loader.  The loader must be specified using its
	     full pathname, and boot_module.xwam must exist.
    	module_to_disassemble:
	     This is a developer's option.
   	     The -d flag tells XSB to act as a disassembler.
	     The -B flag specifies the module to disassemble.
	cmd_loop_driver:
	     The top-level command loop driver to be used instead of the
	     standard one.  Usually needed when XSB is run as a server.

                    -B : specify the boot module to use in lieu of the standard loader
                    -D : Sets top-level command loop driver to replace the default
                    -t : trace execution at the SLG-WAM instruction level
                         (for this to work, build XSB with the --debug option)
                    -d : disassemble the module specified by the -B flag to stdout and exit
         -v, --version : print the version and configuration information about XSB
            -h, --help : print this help message

Flags: 
      	memory management flags:
	     -c tcpsize [unit] | -m glsize [unit] | -o complsize [unit] 
             | -u pdlsize [unit] | -r | -g gc_type
          unit: k/K memory in kilobytes; m/M in megabytes; g/G in gigabytes

               -e goal : evaluate goal when XSB starts up
                    -p : enable Prolog profiling through use of profile_call/1
                    -l : the interpreter prints unbound variables using letters
            --nobanner : don't show the XSB banner on startup
           --quietload : don't show the `module loaded' messages
            --noprompt : don't show prompt (for non-interactive use)
                    -S : set default tabling method to call-subsumption
 --max_subgoal_size  N : set maximum tabled subgoal size to N (default is maximum integer)
--max_subgoal_action A : set action on maximum subgoal size: e(rror)/a(bstract)/w(arn)
         --max_tries N : allow up to N tries for interning terms
       --max_threads N : maintain information for up to N threads (MT engine only)
       --max_mqueues N : allow up to N message queues (MT engine only)
   --shared_predicates : make predicates thread-shared by default
            -g gc_type : choose heap garbage collection ("indirection","none" or "copying")
           -c N [unit] : allocate N units (def. KB) for the trail/choice-point stack
           -m N [unit] : allocate N units (def. KB) for the local/global stack
           -o N [unit] : allocate N units (def. KB) for the SLG completion stack
           -u N [unit] : allocate N units (def. KB) for the SLG unification stack
                    -r : turn off automatic stack expansion
                    -T : print a trace of each called predicate

