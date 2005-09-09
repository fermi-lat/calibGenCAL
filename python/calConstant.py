"""
Global calorimeter constant values
"""


__facility__      = "Online"
__abstract__      = "Global calorimeter constant values"
__author__        = "D.L. Wood"
__date__          = "$Date: 2005/07/28 22:38:29 $"
__version__       = "$Revision: 1.5 $, $Author: fewtrell $"
__release__       = "$Name:  $"
__credits__       = "NRL code 7650"




# Boolean string to type mapping.

CAL_BOOLEAN = {'FALSE':False,'TRUE':True}


# Suite statii

SUITE_STATUS = ('FAILED','PASSED','PASSED Conditionally','FAILED','FAILED')
SUITE_CSTATUS = ('<b><font color="Red">FAILED</font></b>','<b><font color="Green">PASSED</font></b>','<b><font color="Orange">PASSED Conditionally</font></b>','<b><font color="Red">ABORTED</font></b>','<b><font color="Red">Terminated</font></b>','<b><font color="Orange">PASSED with Exceptions</font></b>')


# Calorimeter face number to name mapping.

CFACE = ('X+','Y+','X-','Y-')

# Calorimeter side number to name mapping.

CXY = ('X','Y','X','Y')

# Calorimter log end number to name mapping

CPM = ('-','+','-/+')

# Calorimeter range number to name mapping

CRNG = ('LEX8','LEX1','HEX8','HEX1')

# Calorimeter symbolic names for energy range indicies

CRNG_LEX8 = 0
CRNG_LEX1 = 1
CRNG_HEX8 = 2
CRNG_HEX1 = 3

# Calorimeter layer number to string mapping

CLAYER = ('0','1','2','3')

# Calorimeter row number to string mapping

CROW = ('X0','X1','X2','X3','Y0','Y1','Y2','Y3')

# Calorimter log number to string mapping

CLOG = ('0','1','2','3','4','5','6','7','8','9','10','11')

# Calorimeter energy range number to name mapping

CLEHE = ('LE','HE', 'BOTH')

# Calorimeter DAC range to name mapping

CDAC = ('FINE', 'COARSE')

# Number of calorimeter faces

NUM_FACE = 8

# Number of calorimeter layers

NUM_LAYER = 8

# Number of calorimeter rows

NUM_ROW = 8

# Number of calorimeter ends

NUM_END = 2

# Number of calorimeter FE's

NUM_FE = 12

# Number of possible calorimeters TEM's

NUM_TEM = 16


