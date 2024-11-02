The atexit.c source has been temporarily moved here until it can be fixed.
The Portfile has been removing it all along, so this just makes the local
build consistent with the port build.  Since it's been disabled since
its inception in Apr-2022, it clearly hasn't been missed much.  Note that
the relevant entries in add_symbols.c remain present, but that's also true
of the way the Portfile was acting.

It would be highly desirable to have reasonable tests for it before
fixing and reactivating it.

The Portfile block that this replaces (based on the old filename) is:

pre-patch {
    # until upstream can be fixed, do not include atexit symbols
    # under certain circumstances, infinite recursive loops can form
    delete ${worksrcpath}/src/macports_legacy_atexit.c
}
