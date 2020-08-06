## File:      packages/curl/NMakefile.mak
## Author(s): kifer
## Contact:   xsb-contact@cs.sunysb.edu
## 
## Copyright (C) The Research Foundation of SUNY, 2010-2011
## 
## XSB is free software; you can redistribute it and/or modify it under the
## terms of the GNU Library General Public License as published by the Free
## Software Foundation; either version 2 of the License, or (at your option)
## any later version.
## 
## XSB is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
## more details.
## 
## You should have received a copy of the GNU Library General Public License
## along with XSB; if not, write to the Free Software Foundation,
## Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##

ALL::
	cd cc
	nmake /nologo /f NMakefile.mak
	if exist ..\curl_info.P del ..\curl_info.P
	copy ..\Misc\curl_init-wind.P  ..\curl_info.P
	cd ..\..

CLEAN::
	cd cc
	nmake /nologo /f NMakefile.mak clean
	cd ..
