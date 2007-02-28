# $Header$
# @file calibGenCAL analysis Makefile - manage generation of all calibGenCAL outputs and accessory files
# @requires GNU Make
# @requires calibGenCAL package CMT environment setup - this means source your $CALIBGENCALROOT/cmt/setup.(csh|sh|bat)


##################
## INPUT FILES ###
##################

## SECTION A: PEDESTALS

## SECTION B: INT-NONLIN (CIDAC2ADC) 

### B1: MUON GAIN INT-NONLIN
# input LE singlex16 digi root event file (muon gain)
INPUT_INL_LE_MUON = #/nfs/farm/g/glast/u34/Integration/rootData/077005401/v6r070329p16/grRoot/digitization-licos-v3r6p2_077005401_digi_DIGI.root
# input HE singlex16 digi root event file (muon gain)
INPUT_INL_HE_MUON = #/nfs/farm/g/glast/u34/Integration/rootData/077005404/v6r070329p16/grRoot/digitization-licos-v3r6p2_077005404_digi_DIGI.root

### B2: FLIGHT GAIN INT-NONLIN
# input LE singlex16 digi root event file (flight gain)
INPUT_INL_LE_FLIGHT = 
# input HE singlex16 digi root event file (flight gain)
INPUT_INL_HE_FLIGHT = 

### B3: 'NEIGHBOR' XTALK
# input 'NeighborXtalk' singlex16 digi root event file (flight gain, le_pulse_only, columnwise)
INPUT_NEIGHBOR_XTALK = /nfs/slac/g/glast/users/glground/bregeon/DIGICALCI/700002700.root

#############
## OPTIONS ##
#############

# set to '-b' if inl data is in broadcast mode, blank if not
INL_MUON_BCAST   = -b
INL_FLIGHT_BCAST = -b


#########################
### AUTOGEN_FILENAMES ###
#########################
INL_MUON_BASE = cidac2adc.$(basename $(notdir ${INPUT_INL_LE_MUON}))
INL_MUON_TXT  = $(addsuffix .txt, ${INL_MUON_BASE})
INL_MUON_XML  = $(addsuffix .xml, ${INL_MUON_BASE})
INL_MUON_VAL  = $(addsuffix .val.root, ${INL_MUON_BASE})

INL_FLIGHT_BASE = cidac2adc.$(basename $(notdir ${INPUT_INL_LE_FLIGHT}))
INL_FLIGHT_TXT  = $(addsuffix .txt, ${INL_FLIGHT_BASE})
INL_FLIGHT_XML  = $(addsuffix .xml, ${INL_FLIGHT_BASE})
INL_FLIGHT_VAL  = $(addsuffix .val.root, ${INL_FLIGHT_BASE})

NEIGHBOR_XTALK_BASE = neighborXtalk.$(basename $(notdir ${INPUT_NEIGHBOR_XTALK}))
NEIGHBOR_XTALK_TXT  = $(addsuffix .txt, ${NEIGHBOR_XTALK_BASE})



#########################
## General ENV SETUP ####
#########################
#DISPLAY =





####################
## GENERIC RULES ###
####################
cidac2adc.%.val.root : cidac2adc.%.xml
	intNonlinVal.sh -r $<

cidac2adc.%.xml : cidac2adc.%.txt
	inlTXT2XML.sh $< $@

cidac2adc.%.txt : cidac2adc.%.adcmean.txt
	smoothINL.exe $< $@




##########################
### TARGET OUTPUTS #######
##########################
all: inl_val neighbor_xtalk

inl : inl_val

inl_val : ${INL_MUON_VAL} ${INL_FLIGHT_VAL}


inl_xml : ${INL_MUON_XML} ${INL_FLIGHT_XML}


inl_txt : ${INL_MUON_TXT} ${INL_FLIGHT_TXT}
${INL_MUON_TXT} : ${INPUT_INL_LE_MUON} ${INPUT_INL_HE_MUON}
	genCIDAC2ADC.exe ${INL_MUON_BCAST} -l ${INPUT_INL_LE_MUON} -h ${INPUT_INL_HE_MUON} ${INL_MUON_BASE}

neighbor_xtalk : ${NEIGHBOR_XTALK_TXT}
${NEIGHBOR_XTALK_TXT} : ${INPUT_NEIGHBOR_XTALK}
	genNeighborXtalk.exe $< ${NEIGHBOR_XTALK_BASE}