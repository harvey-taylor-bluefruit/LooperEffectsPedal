### Plain ASM vs inline ASM
* Inline ASM will be compiled and debugging can only be done through disassembly, where comments cannot be easily viewed
* There are lots of quirks you have to be aware of, thus increasing the liklihood of a bug. See this page: http://www.ethernut.de/en/documents/arm-inline-asm.html
* No need to worry about function prologue / epilogue

### C vs C++ for port-specific code
* C++ :: syntax for member functions is really nice, but relies on name mangling, which is obfuscating when function implementation is in assembler
* For simplicity's and readility's sake, we opted for C & C-style prefix (e.g. *Context_*Switch, vs C++ Context::Switch)
* Public API located in Scheduler root folder, with core code (and internal headers) in a subfolder

### Hacks (yes, I know, smelly)
* If building unit tests project in Windows, you may get an error when trying to Clean Project ("make clean"). The best workaround I've found is to make a copy/link of MinGW's rm executable to "del.exe". Not pretty, but it works.

### Porting guide
* Make sure the MsToTicks method returns the correct value