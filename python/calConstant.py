"""
Global calorimeter constant values
"""


__facility__      = "Online"
__abstract__      = "Global calorimeter constant values"
__author__        = "D.L. Wood"
__date__          = "$Date: 2005/07/27 19:46:41 $"
__version__       = "$Revision: 1.4 $, $Author: fewtrell $"
__release__       = "$Name: v3r6p15 $"
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

# Caliormeter charge inject DAC name to value mapping

CI_IDX = {'L':0x18,'H':0x28,'B':0x38}

# Caliormeter CCC ASIC name to number mapping

CC_IDX = {'X+':0,'Y+':1,'X-':2,'Y-':3,'Al':255}

# Caliormeter CRC ASIC name to number mapping

RC_IDX = {'0':0,'1':1,'2':2,'3':3,'A':255}

# Caliormeter CFE ASIC name to number mapping

FE_IDX = {'0':0,'1':1,'2':2,'3':3,'4':4,'5':5,'6':6,'7':7,'8':8,'9':9,'10':10,'11':11,'All':255}

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

# Calorimeter tower number to TEM name mapping

CTEM = ('TWR0','TWR1','TWR2','TWR3','TWR4','TWR5','TWR6','TWR7','TWR8','TWR9','TWR10','TWR11','TWR12','TWR13','TWR14','TWR15')

# Calorimeter energy range number to name mapping

CLEHE = ('LE','HE', 'BOTH')

# Calorimeter DAC range to name mapping

CDAC = ('FINE', 'COARSE')

# Nominal calorimeter system clock frequency

NOMCLOCKFREQ = 20000000.

# Value to string mapping

CYESNO = ('YES','NO')

# Temperature region names

TEMP_REGIONS = ('HOT','ROOM','COLD')
