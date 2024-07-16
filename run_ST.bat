@echo off
rem Compile the file Final_TP_HPC.c
gcc -o Final_TP_HPC Final_TP_HPC.c
rem Check if the compilation was successful
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b %errorlevel%
)
rem Execute the generated executable
Final_TP_HPC.exe
rem Keep the command prompt open after execution
pause
