del out.exe
del 1.ll
set LLVMPATH=C:\dev\TypeScriptCompiler\3rdParty\llvm\release\bin
set TSCPATH=C:\dev\TypeScriptCompiler\__build\tsc\bin
set LIBPATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\lib\x64"
set SDKPATH="C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64"
set UCRTPATH="C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64"
%TSCPATH%\tsc.exe --emit=llvm C:\temp\1.ts 2>1.ll
%TSCPATH%\tsc.exe --emit=jit -dump-object-file -object-filename=out.o C:\temp\1.ts
%LLVMPATH%\lld.exe -flavor link out.o /libpath:%LIBPATH% /libpath:%SDKPATH% /libpath:%UCRTPATH% /defaultlib:libcmt.lib libvcruntime.lib
del *.o

echo "RUN:..."
out.exe
