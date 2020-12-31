@echo off

git submodule foreach git checkout master
git submodule foreach git pull

pause
