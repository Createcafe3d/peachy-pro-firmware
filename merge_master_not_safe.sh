#!/bin/bash
echo ----------------------
echo      MERGING
echo ----------------------

git checkout not_safe
git merge master
git push
git checkout master

echo ----------------------
echo    DONE MERGING
echo ----------------------
