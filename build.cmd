@echo off

REM Compiler to use.
SET compiler=clang++

REM Strip to use.
SET strip=llvm-strip

REM Strip flags. Removes .pdata required for SEH.
SET strip_flags=--strip-all --remove-section=.pdata

REM Projects source files.
SET source_files=src/main.cpp

REM Binary output path.
SET binary_path=bin/tinyinjector.exe

REM Required libraries
SET libraries=-luser32 -lkernel32 -lntdll -lshell32

REM Compilation flags.
set flags=--target=x86_64-pc-windows-gnu -mwindows -Wall -std=c++23 -nostdlib -nostartfiles -fno-rtti -fno-exceptions -fuse-ld=lld -Wl,/SUBSYSTEM:CONSOLE -e entry_point  -fno-use-cxa-atexit

REM Debug only flags.
SET flags_debug=-g

REM Release only flags.
SET flags_release=-g0 -O3 -DNDEBUG -flto -fomit-frame-pointer -fno-unwind-tables -mno-stack-arg-probe -Wl,/release -Wl,/stub:none -Wl,/safeseh:no -Wl,--strip-all

REM Apply correct flags.
IF "%1"=="debug" (
    SET flags=%flags% %flags_debug%
) ELSE (
    SET flags=%flags% %flags_release%
)

@echo on

%compiler% %flags% %source_files% %libraries% -o %binary_path%

@echo off
REM Skip stripping if debug.
IF "%1"=="debug" GOTO END

@echo on
%strip% %strip_flags% %binary_path%

:END
