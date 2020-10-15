Command line used to find this crash:

afl-fuzz.exe -i input -o solfedge-round-1 -D ..\..\..\..\DynamoRIO-Windows-8.0.0-1\DynamoRIO-Windows-8.0.0-1\bin64 -t 20000 -- -coverage_module solfedge-project.exe -target_module solfedge-project.exe -target_offset 0x1230 -nargs 2 -- solfedge-project.exe 804619 @@

If you can't reproduce a bug outside of afl-fuzz, be sure to set the same
memory limit. The limit used for this fuzzing session was 0 B.

Need a tool to minimize test cases before investigating the crashes or sending
them to a vendor? Check out the afl-tmin that comes with the fuzzer!

Found any cool bugs in open-source tools using afl-fuzz? If yes, please drop
me a mail at <lcamtuf@coredump.cx> once the issues are fixed - I'd love to
add your finds to the gallery at:

  http:\\lcamtuf.coredump.cx\afl\

Thanks :-)
