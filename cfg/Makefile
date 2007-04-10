# $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/cfg/Makefile,v 1.4 2007/03/06 17:40:37 fewtrell Exp $
# @file calibGenCAL analysis Makefile - manage generation of all calibGenCAL outputs and accessory files
# @requires GNU Make
# @requires calibGenCAL package CMT environment setup - this means source your $CALIBGENCALROOT/cmt/setup.(csh|sh|bat)


##################
## INPUT FILES ###
##################

## SECTION A: PEDESTALS ##

### A1: MUON GAIN PEDESTALS

MUPED_INPUT_ROOT_FILES = muon_rootfile_list.txt

# options 'PERIODIC' 'EXTERNAL' 'ALL' (default = 'PERIODIC')
MUPED_EVENT_FILTER     = PERIODIC

# 1000 events minimum per hist is sufficient
MUPED_NENTRIES_PERHIST = 1000



## SECTION B: INT-NONLIN (CIDAC2ADC) ##

### B1: MUON GAIN INT-NONLIN
# input LE singlex16 digi root event file (muon gain, calibGain=on, calibGen element#201)
INPUT_INL_MUON_LE = /nfs/farm/g/glast/u34/Integration/rootData/077005401/v6r070329p16/grRoot/digitization-licos-v3r6p2_077005401_digi_DIGI.root
# input HE singlex16 digi root event file (muon gain), calibGain=off, calibGen element#204)
INPUT_INL_MUON_HE = /nfs/farm/g/glast/u34/Integration/rootData/077005404/v6r070329p16/grRoot/digitization-licos-v3r6p2_077005404_digi_DIGI.root

### B2: FLIGHT GAIN INT-NONLIN
# input LE singlex16 digi root event file (flight gain)
INPUT_INL_LE_FLIGHT = 
# input HE singlex16 digi root event file (flight gain)
INPUT_INL_HE_FLIGHT = 

### B3: 'NEIGHBOR' XTALK
# input 'NeighborXtalk' singlex16 digi root event file (flight gain, le_pulse_only, columnwise)
INPUT_NEIGHBOR_XTALK = /nfs/slac/g/glast/users/glground/bregeon/DIGICALCI/700002700.root



## SECTION C: OPTICAL RESPONSE (MUON) ##

### C1: MEVPERDAC / ASYM COMBINED USING TRACKER TRACKS
# first digi root input file for muon optical response calibration
# (this file will be used for output filename generation) - the full list of files should be found in CGC_MUON_CFG file
INPUT_FIRST_MUOPT_ROOT = /nfs/farm/g/glast/u34/Integration/rootData/077003627/v6r070329p16/grRoot/digitization-licos-v3r6p2_077003627_digi_DIGI.root
# first digi root input file for muon optical response calibration
# (this file will be used for output filename generation) - the full list of files should be found in CGC_MUON_CFG file

INPUT_FIRST_MUOPT_SVAC = /nfs/farm/g/glast/u34/Integration/rootData/077003627/v6r070329p16/calib-v1r0/svacTuple/emRootv0r0/svacTuple-v3r6p2_077003627_svac_svac.root



#############
## OPTIONS ##
#############

# set to '-b' if inl data is in broadcast mode, blank if not
INL_BCAST   = -b
INL_MUON_FLIGHT_BCAST = -b


#########################
### AUTOGEN_FILENAMES ###
#########################
INL_PFX     = cidac2adc
PED_PFX     = calPeds
ASYM_PFX    = calAsym
MPD_PFX     = calMPD
MUSLOPE_PFX = muSlope
ADC2NRG_PFX = adc2nrg

INL_MUON_BASE = ${INL_PFX}.$(basename $(notdir ${INPUT_INL_MUON_LE}))
INL_MUON_TXT  = $(addsuffix .txt, ${INL_MUON_BASE})
INL_MUON_XML  = $(addsuffix .xml, ${INL_MUON_BASE})
INL_MUON_VAL  = $(addsuffix .val.root, ${INL_MUON_BASE})

INL_FLIGHT_BASE = ${INL_PFX}.$(basename $(notdir ${INPUT_INL_LE_FLIGHT}))
INL_FLIGHT_TXT  = $(addsuffix .txt, ${INL_FLIGHT_BASE})
INL_FLIGHT_XML  = $(addsuffix .xml, ${INL_FLIGHT_BASE})
INL_FLIGHT_VAL  = $(addsuffix .val.root, ${INL_FLIGHT_BASE})

NEIGHBOR_XTALK_BASE = neighborXtalk.$(basename $(notdir ${INPUT_NEIGHBOR_XTALK}))
NEIGHBOR_XTALK_TXT  = $(addsuffix .txt, ${NEIGHBOR_XTALK_BASE})

PED_MUON_BASE = ${PED_PFX}.$(basename $(notdir ${FIRST_PED_ROOT_INPUT}))
PED_MUON_TXT  = $(addsuffix .txt, ${PED_MUON_BASE})
PED_MUON_XML  = $(addsuffix .xml, ${PED_MUON_BASE})
PED_MUON_VAL  = $(addsuffix .val.root, ${PED_MUON_BASE})

ASYM_MUON_BASE = ${ASYM_PFX}.$(basename $(notdir ${INPUT_FIRST_MUOPT_ROOT}))
ASYM_MUON_TXT  = $(addsuffix .txt, ${ASYM_MUON_BASE})
ASYM_MUON_XML  = $(addsuffix .xml, ${ASYM_MUON_BASE})
ASYM_MUON_VAL  = $(addsuffix .val.root, ${ASYM_MUON_BASE})

MPD_MUON_BASE = ${MPD_PFX}.$(basename $(notdir ${INPUT_FIRST_MUOPT_ROOT}))
MPD_MUON_TXT  = $(addsuffix .txt, ${MPD_MUON_BASE})
MPD_MUON_XML  = $(addsuffix .xml, ${MPD_MUON_BASE})
MPD_MUON_VAL  = $(addsuffix .val.root, ${MPD_MUON_BASE})

MUSLOPE_BASE = ${MUSLOPE_PFX}.$(basename $(notdir ${INPUT_FIRST_MUOPT_ROOT}))
MUSLOPE_XML  = $(addsuffix .xml, ${MUSLOPE_BASE})
MUSLOPE_VAL  = $(addsuffix .val.root, ${MUSLOPE_BASE})

ADC2NRG_BASE = ${ADC2NRG_PFX}.$(basename $(notdir ${INPUT_FIRST_MUOPT_ROOT}))
ADC2NRG_XML  = $(addsuffix .xml, ${ADC2NRG_BASE})
ADC2NRG_VAL  = $(addsuffix .val.root, ${ADC2NRG_BASE})

#########################
## General ENV SETUP ####
#########################
#DISPLAY =

####################
## GENERIC RULES ###
####################
${INL_PFX}.%.val.root : ${INL_PFX}.%.xml
	intNonlinVal.sh -R $@ $<

${INL_PFX}.%.xml : ${INL_PFX}.%.txt
	inlTXT2XML.sh $< $@

# ${INL_PFX}.%.txt : ${INL_PFX}.%.adcmean.txt
# 	smoothINL.exe $< $@

${PED_PFX}.%.xml : ${PED_PFX}.%.txt
	pedTXT2XML.sh $< $@

${PED_PFX}.%.val.root : ${PED_PFX}.%.xml
	pedVal.sh -R $@ $<


${ASYM_PFX}.%.xml : ${ASYM_PFX}.%.txt
	asymTXT2XML.sh $< $@

${ASYM_PFX}.%.val.root : ${ASYM_PFX}.%.xml
	asymVal.sh -R $@ $<


${MPD_PFX}.%.xml : ${MPD_PFX}.%.txt
	mpdTXT2XML.sh $< $@

${MPD_PFX}.%.val.root : ${MPD_PFX}.%.xml
	mevPerDacVal.sh -R $@ $<

${MUSLOPE_PFX}.%.val.root : ${MUSLOPE_PFX}.%.xml
	muSlopeVal.sh -R $@ $<

${ADC2NRG_PFX}.%.val.root : ${ADC2NRG_PFX}.%.xml
	adc2nrgVal.sh -R $@ $<


##########################
### TARGET OUTPUTS #######
##########################
all: ped inl muopt

inl : inl_muon_val

inl_muon_val : ${INL_MUON_VAL}

inl_muon_xml : ${INL_MUON_XML} 

inl_muon_txt : ${INL_MUON_TXT} 

${INL_MUON_TXT} : ${INPUT_INL_MUON_LE} ${INPUT_INL_MUON_HE}
	genCIDAC2ADC.exe ${CGC_MUON_CFG}

neighbor_xtalk : ${NEIGHBOR_XTALK_TXT}

${NEIGHBOR_XTALK_TXT} : ${INPUT_NEIGHBOR_XTALK}
	genNeighborXtalk.exe $< ${NEIGHBOR_XTALK_BASE}

peds : ped

ped : ped_muon_val

ped_muon_val : ${PED_MUON_VAL}

ped_muon_xml : ${PED_MUON_XML}

ped_muon_txt : ${PED_MUON_TXT}

${PED_MUON_TXT} :
	genMuonPed.exe ${CGC_MUON_CFG}




muopt : muopt_val

muopt_val : ${ASYM_MUON_VAL} ${MPD_MUON_VAL} ${MUSLOPE_VAL} ${ADC2NRG_VAL}

asym_muon_val : ${ASYM_MUON_VAL}

mpd_muon_val : ${MPD_MUON_VAL}

muslope_val : ${MUSLOPE_VAL}

adc2nrg_val : ${ADC2NRG_VAL}



muopt_xml : ${ASYM_MUON_XML} ${MPD_MUON_XML} ${MUSLOPE_XML}

asym_muon_xml : ${ASYM_MUON_XML}

muslope_xml : ${MUSLOPE_XML}

mpd_muon_xml : ${MPD_MUON_XML}

# adc2nrg.xml & muslope.xml are created simultaneously
${ADC2NRG_XML} : ${MUSLOPE_XML}

${MUSLOPE_XML} :  ${INL_MUON_XML} ${ASYM_MUON_XML} ${MPD_MUON_XML}
	genADC2NRG.sh ${INL_MUON_XML} ${ASYM_MUON_XML} ${MPD_MUON_XML} ${ADC2NRG_XML} ${MUSLOPE_XML}



asym_muon_txt : ${ASYM_MUON_TXT}

mpd_muon_txt : ${MPD_MUON_TXT}

muopt_txt : ${ASYM_MUON_TXT} ${MPD_MUON_TXT}


# asym.txt & mpd.txt are created simultaneously
${ASYM_MUON_TXT} : ${MPD_MUON_TXT}

${MPD_MUON_TXT} : ${PED_MUON_TXT} ${INL_MUON_TXT}
	genMuonCalibTkr.exe ${CGC_MUON_CFG}
