project('vac-enc', 'c',
    version : '0.0.2',
    default_options : [              
          'warning_level=2',             
          'c_std=gnu99',                 
        ],                               
        license : 'GPLv3',                     
    )


libopusenc_dep = dependency('libopusenc')
opus_dep = dependency('opus')
soxr_dep = dependency('soxr')
openmp_dep = dependency('openmp')

deps = [libopusenc_dep, 
    opus_dep, 
    soxr_dep,
    openmp_dep
    ]

executable('vac-enc', 
    ['src/main.c', 'src/wavreader.c', 'src/wavreader.h'], 
    dependencies : deps)
