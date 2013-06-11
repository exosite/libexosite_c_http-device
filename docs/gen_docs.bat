
:: parameter %1 is the source directory
:: parameter %2 is the image output directory

:: set tools
SET GRAPHVIZ_DOT="C:\Program Files (x86)\Graphviz2.30\bin\dot.exe"
SET DOXYGEN_EXE="C:\Program Files\doxygen\bin\doxygen.exe"
SET PLANTUML_JAR=C:\inPath\plantuml.jar

:: set source dir
SET SOURCE_DIR=..\src

:: set image output folder for plantuml / image source folder for doxygen
SET DOC_IMG_PATH=..\docs\html\

:: change to source dir and save old dir on stack
PUSHD "%SOURCE_DIR%"

:: call plantuml.jar for current dir
java -jar %PLANTUML_JAR% -v -o %DOC_IMG_PATH% "./**.(c|cpp|dox|h)"

:: change to saved dir
POPD

:: call doxygen for current dir (doxyfile)
%DOXYGEN_EXE% ..\docs\doxyfile

