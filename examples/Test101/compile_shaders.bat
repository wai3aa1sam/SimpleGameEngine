@REM #change current directory to this file
@%~d0
@cd %~dp0

@rem set comp=..\..\build\SimpleGameEngine-x64-windows\src\render\shader_compiler\Debug\sge_shader_compiler.exe
@rem %comp%

set PROJECT_ROOT=%~dp0
set SGE_ROOT=%~dp0..\..
set COMPILE_SHADER_PATH=%SGE_ROOT%\built-in\script\sge_compile_shader

@rem @echo --- SGE_ROOT --- %SGE_ROOT%
@rem @echo --- PROJECT_ROOT --- %PROJECT_ROOT%
@rem @echo --- COMPILE_SHADER_PATH --- %COMPILE_SHADER_PATH%

pushd %COMPILE_SHADER_PATH%

python sge_compile_shader.py %PROJECT_ROOT%

popd

pause