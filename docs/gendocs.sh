#!/bin/sh
# Create *.png files from text @startuml code and output *.png images to ./doc-src/images folder
# Recusively search from current folder scanning *.c, *.cpp, *.h, and *.dox files
java -Djava.awt.headless=true -jar /usr/share/plantuml/plantuml.jar -v -o "../../docs/html/"  "../exosite-lib/src/exosite_c_libraries.dox"
doxygen Doxyfile
