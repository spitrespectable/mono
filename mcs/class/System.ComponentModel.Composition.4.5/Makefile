thisdir = class/System.ComponentModel.Composition.4.5
SUBDIRS = 
include ../../build/rules.make

LIBRARY = System.ComponentModel.Composition.dll
LIB_REFS = System System.Core

NO_RESGEN := $(filter basic build, $(PROFILE))
ifndef NO_RESGEN
  RESOURCE_DEFS = Microsoft.Internal.Strings,src/ComponentModel/Strings.resx
endif

KEYFILE = ../ecma.pub
LIB_MCS_FLAGS = -d:CLR40 -d:USE_ECMA_KEY,FEATURE_REFLECTIONCONTEXT,FEATURE_REFLECTIONFILEIO,FEATURE_SERIALIZATION,FEATURE_SLIMLOCK -nowarn:219,414 -nowarn:436


CLEAN_FILES += $(STRING_MESSAGES)

EXTRA_DISTFILES = \
	src/ComponentModel/Strings.resx

include ../../build/library.make


