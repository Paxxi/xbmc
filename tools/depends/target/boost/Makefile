include ../../Makefile.include
DEPS= ../../Makefile.include fix_shared_ptr_multithread.patch Makefile

#Headers Only!
LIBNAME=boost
VERSION=1_57_0
SOURCE=$(LIBNAME)_$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
ifeq (arm, $(findstring arm, $(CPU)))
	cd $(PLATFORM); patch -p0 < ../fix_shared_ptr_multithread.patch
endif

.installed-$(PLATFORM): $(PLATFORM)
	cd $(PLATFORM); touch .installed-$(PLATFORM)

clean:
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
