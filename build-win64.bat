@rem
@rem   Copyright 2021 MacKenzie Strand
@rem
@rem   Licensed under the Apache License, Version 2.0 (the "License");
@rem   you may not use this file except in compliance with the License.
@rem   You may obtain a copy of the License at
@rem
@rem       http://www.apache.org/licenses/LICENSE-2.0
@rem
@rem   Unless required by applicable law or agreed to in writing, software
@rem   distributed under the License is distributed on an "AS IS" BASIS,
@rem   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem   See the License for the specific language governing permissions and
@rem   limitations under the License.
@rem
@echo off

@rem Update Libraries
:get_libraries
echo Getting Libraries...
if exist libs\Win64\ (
	cd libs\Win64
	git pull
	cd ..\..\
) else (
	if not exist libs\ (mkdir libs)
	if not exist libs\Win64\ (mkdir libs\Win64)
	goto :prompt_download
)

@rem Get Python Dependencies
:get_python_deps
echo Getting Python Dependencies...
call libs\Win64\PythonInstalls.bat

@rem Run CMake Configure
:configure_cmake
echo Configuring CMake...
mkdir build
cmake -S . -B build\
pause
exit 0

@rem Prompt GitLFS Clone
:prompt_download
set "DOWNLOAD=y"
set /p "DOWNLOAD=Download Libraries? (Uses GitLFS Quota) [y/n]"
if /i "%DOWNLOAD%" equ "n" (
	echo You will need to download a copy of the libs repo manually, visit the readme...
	pause
	exit 1
)
if /i "%DOWNLOAD%" equ "y" (
	@rem Todo: Better file hosting solution!
	git clone https://github.com/KenzieMac130/CitrusDeps-Win64.git libs\Win64
	goto :get_python_deps
)
goto :prompt_download