@ECHO OFF

pushd %~dp0

REM Minimal Sphinx make.bat for Windows.

if "%SPHINXBUILD%" == "" (
	set SPHINXBUILD=sphinx-build
)
if "%SPHINXOPTS%" == "" (
	set SPHINXOPTS=-W --keep-going
)
set SOURCEDIR=.
set BUILDDIR=_build

%SPHINXBUILD% >NUL 2>NUL
if errorlevel 9009 (
	echo.
	echo.The 'sphinx-build' command was not found. Install it with:
	echo.    pip install -r requirements.txt
	echo.
	exit /b 1
)

if "%1" == "" goto help
if "%1" == "html" goto html
if "%1" == "linkcheck" goto linkcheck
if "%1" == "clean" goto clean

%SPHINXBUILD% -M %1 %SOURCEDIR% %BUILDDIR% %SPHINXOPTS%
goto end

:help
%SPHINXBUILD% -M help %SOURCEDIR% %BUILDDIR% %SPHINXOPTS%
goto end

:html
%SPHINXBUILD% -b html %SOURCEDIR% %BUILDDIR%\html %SPHINXOPTS%
goto end

:linkcheck
%SPHINXBUILD% -b linkcheck %SOURCEDIR% %BUILDDIR%\linkcheck %SPHINXOPTS%
goto end

:clean
rmdir /S /Q %BUILDDIR%
goto end

:end
popd
