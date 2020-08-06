## File:      packages/pcre/NMakefile64.mak
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
	nmake /f NMakefile64.mak
	cd ..
	if exist pcre_info.P del pcre_info.P
	copy Misc\pcre_init-wind.P  pcre_info.P
	cd ..

CLEAN::
	cd cc
	nmake /nologo /f NMakefile64.mak clean
	cd ..
