use ExtUtils::MakeMaker;
WriteMakefile(
    'NAME' => 'simstring',      # Name of package
    'CC' => 'g++',
    'LD' => 'g++',
    'INC' => '-I../../include', # Include dirs
    'OBJECT' => 'export.o export_wrap.o'  # Object files
    );

