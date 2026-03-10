@echo off
REM  pwsh -Command "Set-Location '%cd%/build/'"
wt -d "%cd%/build/"
gvim ./src/main.c
