add_executable(vac-enc
    main.c
    wavreader.c
    wavreader.h)

target_link_libraries(vac-enc PUBLIC
        PkgConfig::dep1
        PkgConfig::dep2
	PkgConfig::dep3)
