#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# set RPATH-type linker instructions here so they can be used in the shared
# version and in the mixed (static nss libs/shared NSPR libs) version.

ifeq ($(OS_ARCH), SunOS) 
ifeq ($(BUILD_SUN_PKG), 1)
ifeq ($(USE_64), 1)
EXTRA_SHARED_LIBS += -R '$$ORIGIN/../lib:/usr/lib/mps/secv1/64:/usr/lib/mps/64'
else
EXTRA_SHARED_LIBS += -R '$$ORIGIN/../lib:/usr/lib/mps/secv1:/usr/lib/mps'
endif
else
EXTRA_SHARED_LIBS += -R '$$ORIGIN/../lib'
endif
endif

ifeq ($(OS_ARCH), Linux)
ifeq ($(BUILD_SUN_PKG), 1)
ifeq ($(USE_64), 1)
EXTRA_SHARED_LIBS += -Wl,-rpath,'$$ORIGIN/../lib64:/opt/sun/private/lib64:$$ORIGIN/../lib'
else
EXTRA_SHARED_LIBS += -Wl,-rpath,'$$ORIGIN/../lib:/opt/sun/private/lib'
endif
else
ifeq ($(USE_64), 1)
EXTRA_SHARED_LIBS += -Wl,-rpath,'$$ORIGIN/../lib64:$$ORIGIN/../lib'
else
EXTRA_SHARED_LIBS += -Wl,-rpath,'$$ORIGIN/../lib'
endif
endif
endif

ifeq ($(OS_ARCH), HP-UX) 
ifeq ($(OS_TEST), ia64)
EXTRA_SHARED_LIBS += -Wl,+b,'$$ORIGIN/../lib'
else
# pa-risc
ifeq ($(USE_64), 1)
EXTRA_SHARED_LIBS += \
-Wl,+b,'$$ORIGIN/../../lib/pa20_64:$$ORIGIN/../../lib/64:$$ORIGIN/../lib'
else
EXTRA_SHARED_LIBS += -Wl,+b,'$$ORIGIN/../lib'
endif
endif
endif

ifdef NSS_DISABLE_DBM
DBMLIB = $(NULL)
else
DBMLIB = $(DIST)/lib/$(LIB_PREFIX)dbm.$(LIB_SUFFIX) 
endif

ifdef USE_STATIC_LIBS

DEFINES += -DNSS_USE_STATIC_LIBS
# $(PROGRAM) has explicit dependencies on $(EXTRA_LIBS)
CRYPTOLIB=$(SOFTOKEN_LIB_DIR)/$(LIB_PREFIX)freebl.$(LIB_SUFFIX)

PKIXLIB = \
	$(DIST)/lib/$(LIB_PREFIX)pkixtop.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixutil.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixsystem.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixcrlsel.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixmodule.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixstore.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixparams.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixchecker.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixpki.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixtop.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixresults.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkixcertsel.$(LIB_SUFFIX)

# can't do this in manifest.mn because OS_ARCH isn't defined there.
ifeq (,$(filter-out WINNT WINCE,$(OS_ARCH))) 

EXTRA_LIBS += \
	$(DIST)/lib/$(LIB_PREFIX)smime.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)ssl.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nss.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)ssl.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)sectool.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkcs12.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkcs7.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)certhi.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)cryptohi.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pk11wrap.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)certdb.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)softokn.$(LIB_SUFFIX) \
	$(CRYPTOLIB) \
	$(DIST)/lib/$(LIB_PREFIX)nsspki.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nssdev.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nssb.$(LIB_SUFFIX) \
	$(PKIXLIB) \
	$(DBMLIB) \
	$(DIST)/lib/$(LIB_PREFIX)$(SQLITE_LIB_NAME).$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nssutil3.$(LIB_SUFFIX) \
	$(NSPR_LIB_DIR)/$(NSPR31_LIB_PREFIX)plc4.$(LIB_SUFFIX) \
	$(NSPR_LIB_DIR)/$(NSPR31_LIB_PREFIX)plds4.$(LIB_SUFFIX) \
	$(NSPR_LIB_DIR)/$(NSPR31_LIB_PREFIX)nspr4.$(LIB_SUFFIX) \
	$(NULL)

# $(PROGRAM) has NO explicit dependencies on $(OS_LIBS)
#OS_LIBS += \
	wsock32.lib \
	winmm.lib \
	$(NULL)
else

EXTRA_LIBS += \
	$(DIST)/lib/$(LIB_PREFIX)smime.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)ssl.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nss.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)ssl.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)sectool.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkcs12.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pkcs7.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)certhi.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pk11wrap.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)cryptohi.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)certhi.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nsspki.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pk11wrap.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)softokn.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)certdb.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nsspki.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nssdev.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)nssb.$(LIB_SUFFIX) \
	$(CRYPTOLIB) \
	$(DBMLIB) \
	$(PKIXLIB) \
	$(DIST)/lib/$(LIB_PREFIX)nss.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)pk11wrap.$(LIB_SUFFIX) \
	$(DIST)/lib/$(LIB_PREFIX)certhi.$(LIB_SUFFIX) \
	$(NULL)

ifeq ($(OS_ARCH), AIX) 
EXTRA_SHARED_LIBS += -brtl 
endif

# $(PROGRAM) has NO explicit dependencies on $(EXTRA_SHARED_LIBS)
# $(EXTRA_SHARED_LIBS) come before $(OS_LIBS), except on AIX.
EXTRA_SHARED_LIBS += \
	-L$(DIST)/lib \
	-l$(SQLITE_LIB_NAME) \
	-L$(NSSUTIL_LIB_DIR) \
	-lnssutil3 \
	-L$(NSPR_LIB_DIR) \
	-lplc4 \
	-lplds4 \
	-lnspr4 \
	$(NULL)
endif

ifeq ($(OS_TARGET), SunOS)
OS_LIBS += -lbsm
endif

else # USE_STATIC_LIBS
# can't do this in manifest.mn because OS_ARCH isn't defined there.
ifeq (,$(filter-out WINNT WINCE,$(OS_ARCH))) 

# $(PROGRAM) has explicit dependencies on $(EXTRA_LIBS)
EXTRA_LIBS += \
	$(DIST)/lib/$(LIB_PREFIX)sectool.$(LIB_SUFFIX) \
	$(DIST)/lib/$(IMPORT_LIB_PREFIX)nssutil3$(IMPORT_LIB_SUFFIX) \
	$(DIST)/lib/$(IMPORT_LIB_PREFIX)smime3$(IMPORT_LIB_SUFFIX) \
	$(DIST)/lib/$(IMPORT_LIB_PREFIX)ssl3$(IMPORT_LIB_SUFFIX) \
	$(DIST)/lib/$(IMPORT_LIB_PREFIX)nss3$(IMPORT_LIB_SUFFIX) \
	$(NSPR_LIB_DIR)/$(NSPR31_LIB_PREFIX)plc4$(IMPORT_LIB_SUFFIX) \
	$(NSPR_LIB_DIR)/$(NSPR31_LIB_PREFIX)plds4$(IMPORT_LIB_SUFFIX) \
	$(NSPR_LIB_DIR)/$(NSPR31_LIB_PREFIX)nspr4$(IMPORT_LIB_SUFFIX) \
	$(NULL)

# $(PROGRAM) has NO explicit dependencies on $(OS_LIBS)
#OS_LIBS += \
	wsock32.lib \
	winmm.lib \
	$(NULL)
else

# $(PROGRAM) has explicit dependencies on $(EXTRA_LIBS)
EXTRA_LIBS += \
	$(DIST)/lib/$(LIB_PREFIX)sectool.$(LIB_SUFFIX) \
	$(NULL)

ifeq ($(OS_ARCH), AIX) 
EXTRA_SHARED_LIBS += -brtl 
endif

# $(PROGRAM) has NO explicit dependencies on $(EXTRA_SHARED_LIBS)
# $(EXTRA_SHARED_LIBS) come before $(OS_LIBS), except on AIX.
EXTRA_SHARED_LIBS += \
	-L$(DIST)/lib \
	-lssl3 \
	-lsmime3 \
	-lnss3 \
	-L$(NSSUTIL_LIB_DIR) \
	-lnssutil3 \
	-L$(NSPR_LIB_DIR) \
	-lplc4 \
	-lplds4 \
	-lnspr4 \
	$(NULL)
endif

endif # USE_STATIC_LIBS

# If a platform has a system zlib, set USE_SYSTEM_ZLIB to 1 and
# ZLIB_LIBS to the linker command-line arguments for the system zlib
# (for example, -lz) in the platform's config file in coreconf.
ifndef USE_SYSTEM_ZLIB
ZLIB_LIBS = $(DIST)/lib/$(LIB_PREFIX)zlib.$(LIB_SUFFIX)
endif

JAR_LIBS = $(DIST)/lib/$(LIB_PREFIX)jar.$(LIB_SUFFIX)
