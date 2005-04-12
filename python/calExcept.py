"""
Calorimeter specific exception classes
"""


__facility__      = "Offline"
__abstract__      = "Calorimeter specific exception classes"
__author__        = "D.L.Wood"
__date__          = "$Date: 2005/01/05 20:11:38 $"
__version__       = "$Revision: 1.6 $, $Author: dwood $"
__release__       = "$Name:  $"
__credits__       = "NRL code 7650"


from exceptions import Exception


##########################################################################################
##
## \class calExcept
##
## \brief Top level CAL exception type.
##
## All other CAL exception classes are derived from this type.  All of the CAL exception
## types pass back a string object as the exception parameter which details the cause
## of the error.
##
##########################################################################################

class calExcept(Exception): pass


##########################################################################################
##
## \class calFileOpenExcept
##
## \brief CAL file open exception.
##
##########################################################################################

class calFileOpenExcept(calExcept): pass


##########################################################################################
##
## \class calFileCloseExcept
##
## \brief CAL file close exception.
##
##########################################################################################

class calFileCloseExcept(calExcept): pass


##########################################################################################
##
## \class calFileReadExcept
##
## \brief CAL file read exception.
##
##########################################################################################

class calFileReadExcept(calExcept): pass


##########################################################################################
##
## \class calFileWriteExcept
##
## \brief CAL file write exception.
##
##########################################################################################

class calFileWriteExcept(calExcept): pass


##########################################################################################
##
## \class calFileStatExcept
##
## \brief CAL file stat exception.
##
##########################################################################################

class calFileStatExcept(calExcept): pass


##########################################################################################
##
## \class calFileExecExcept
##
## \brief CAL file execution exception.
##
##########################################################################################

class calFileExecExcept(calExcept): pass


##########################################################################################
##
## \class calDirOpenExcept
##
## \brief CAL directory open exception.
##
##########################################################################################

class calDirOpenExcept(calExcept): pass


##########################################################################################
##
## \class calConfigSectionExcept
##
## \brief CAL config file section exception.
##
##########################################################################################

class calConfigSectionExcept(calExcept): pass


##########################################################################################
##
## \class calConfigOptionExcept
##
## \brief CAL config file option exception.
##
##########################################################################################

class calConfigOptionExcept(calExcept): pass


##########################################################################################
##
## \class calEnvVarExcept
##
## \brief CAL environment variable lookup exception.
##
##########################################################################################

class calEnvVarExcept(calExcept): pass


##########################################################################################
##
## \class calSesVarExcept
##
## \brief CAL session variable lookup exception.
##
##########################################################################################

class calSesVarExcept(calExcept): pass


##########################################################################################
##
## \class calImportExcept
##
## \brief CAL module import exception.
##
##########################################################################################

class calImportExcept(calExcept): pass


##########################################################################################
##
## \class calRegReadExcept
##
## \brief CAL hardware register read exception.
##
##########################################################################################

class calRegReadExcept(calExcept): pass


##########################################################################################
##
## \class calRegWriteExcept
##
## \brief CAL hardware register write exception.
##
##########################################################################################

class calRegWriteExcept(calExcept): pass
