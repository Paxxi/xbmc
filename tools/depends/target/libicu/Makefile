include ../../Makefile.include
DEPS= ../../Makefile.include Makefile

# lib name, version
LIBNAME=icu
VERSION=54.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
ICU_PLATFORM=

ifeq ($(OS),linux)
	ICU_PLATFORM=Linux/GCC
else ifeq ($(OS),android)
	ICU_PLATFORM=Linux/GCC
else ifeq ($(OS),osx)
	ICU_PLATFORM=MacOSX
else ifeq ($(OS),ios)
	ICU_PLATFORM=MacOSX
endif
# configuration settings
CONFIGURE=cp -f $(CONFIG_SUB) $(CONFIG_GUESS) .; \
        ./runConfigureICU $(ICU_PLATFORM) --prefix=$(PREFIX) --disable-tests --disable-samples

LIBDYLIB=$(PLATFORM)/source/lib/lib$(LIBNAME).a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM)/*; mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM)/source; patch -p0 < ../../uconfig_defines.patch
	cd $(PLATFORM)/source; $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)/source

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM)/source install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM)/source clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
