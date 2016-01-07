# Un-exported variables are internal implementation. API variables are put
# into the environment so that they survive recursive make; the variable
# MPSS_METADATA_PREFIX is _also_ API, but it's input-only of course.

mpss_metadata_c__ := $(dir $(lastword $(MAKEFILE_LIST)))mpss-metadata.c
dot_mpss_metadata__ = $(MPSS_METADATA_PREFIX).mpss-metadata

MPSS_COMMIT ?= $(or $(shell sed -ne '2 p' $(dot_mpss_metadata__) 2>/dev/null), \
	$(error MPSS_COMMIT undefined, check MPSS_METADATA_PREFIX))
MPSS_VERSION ?= $(or $(shell sed -ne '1 p' $(dot_mpss_metadata__) 2>/dev/null), \
	$(error MPSS_VERSION undefined, check MPSS_METADATA_PREFIX))
MPSS_BUILDNO ?= 0

# avoid repeated shell invocations
export MPSS_COMMIT := $(MPSS_COMMIT)
export MPSS_VERSION := $(MPSS_VERSION)
export MPSS_BUILDNO := $(MPSS_BUILDNO)
export MPSS_BUILTBY := $(shell echo "`whoami`@`uname -n`")
export MPSS_BUILTON := $(shell date +'%F %T %z')

# to be mixed into $(CFLAGS)
export MPSS_METADATA_CFLAGS := \
	-DMPSS_COMMIT=\"'$(strip $(MPSS_COMMIT))'\" \
	-DMPSS_VERSION=\"'$(strip $(MPSS_VERSION))'\" \
	-DMPSS_BUILDNO=\"'$(strip $(MPSS_BUILDNO))'\" \
	-DMPSS_BUILTBY=\"'$(strip $(MPSS_BUILTBY))'\" \
	-DMPSS_BUILTON=\"'$(strip $(MPSS_BUILTON))'\"

# deprecated: for compatibility with the previous version
export MPSS_METADATA_C := $(mpss_metadata_c__)

# 'brand' as in 'cattle'
export MPSS_METADATA_BRAND := $(MPSS_METADATA_CFLAGS) $(mpss_metadata_c__)
