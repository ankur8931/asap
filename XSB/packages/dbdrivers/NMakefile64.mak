
ALL::
	cd cc
	nmake /f NMakefile64.mak
	cd ..\odbc
	if exist odbc_init.P del odbc_init.P
	copy Misc\odbc_init-wind.P  odbc_init.P
	cd cc
	nmake /f NMakefile64.mak
	cd ..\..\mysql
	if exist mysql_init.P del mysql_init.P
	copy Misc\mysql_init-wind.P  mysql_init.P
	cd cc
# Too damn difficult to link on windows, so skipping these steps
#       nmake /f NMakefile64.mak
	cd ..\..\mysqlembedded
	if exist mysqlembedded_init.P del mysqlembedded_init.P
	copy Misc\mysqlembedded_init-wind.P  mysqlembedded_init.P
	cd cc
#       nmake /f NMakefile64.mak
	cd ..\..


CLEAN::
	cd cc
	nmake /nologo /f NMakefile64.mak clean
	cd ..\mysql\cc
	nmake /nologo /f NMakefile64.mak clean
	cd ..\..\mysqlembedded\cc
	nmake /nologo /f NMakefile64.mak clean
	cd ..\..\odbc\cc
	nmake /nologo /f NMakefile64.mak clean
	cd ..\..
