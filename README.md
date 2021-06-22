# TypeScript Native Compiler
###### Powered by [![LLVM|MLIR](https://llvm.org/img/LLVM-Logo-Derivative-1.png)](https://llvm.org/)

## Compile as JIT

```cmd
tsc --emit=jit hello.ts
```
File ``hello.ts``

```TypeScript
function main() {
    print("Hello World!");
}
```
Result
```
Hello World!
```

## Compile as Binary Executable

### On Windows
File ``tsc-cmpile.bat``
```cmd
rem set %LLVM% and %TSCBIN%
set LLVMPATH=%LLVM%\llvm\release\bin
set TSCPATH=%TSCBIN%\tsc\bin
set VCFLD=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\SDK\ScopeCppSDK\vc15
set LIBPATH=%VCFLD%\VC\lib
set SDKPATH=%VCFLD%\SDK\lib
set LLVMLIBPATH=%LLVM%\llvm\release\lib
%TSCPATH%\tsc.exe --emit=llvm %FILENAME%.ts 2>%FILENAME%.ll
%LLVMPATH%\llc.exe --filetype=obj -o=%FILENAME%.o %FILENAME%.ll
%LLVMPATH%\lld.exe -flavor link %FILENAME%.o /libpath:"%LIBPATH%" /libpath:"%SDKPATH%" "%LIBPATH%\libcmt.lib" "%LIBPATH%\libvcruntime.lib" "%SDKPATH%\kernel32.lib" "%SDKPATH%\libucrt.lib" "%SDKPATH%\uuid.lib"
```
Compile 
```cmd
tsc-cmpile.bat hello
```

Run
```
hello.exe
```

Result
```
Hello World!
```

## Build
### On Windows

First, precompile dependencies

```
prepare_3rdParty.bat 
```

To build TSC binaries:

```
cd tsc
config_tsc_debug.bat
build_tsc_debug.bat
```
