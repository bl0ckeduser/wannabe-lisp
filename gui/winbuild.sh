mkdir gui-build-win32
cp -r final/* gui-build-win32
rm gui-build-win32/run.sh
cp winlaunch.bat gui-build-win32
# get a xulrunner win32 build from e.g. http://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/23.0/runtimes/xulrunner-23.0.en-US.win32.zip
# then unzip it (directly) to get a "xulrunner" directory
cp -r xulrunner gui-build-win32/code/xulrunner-win32
unix2dos gui-build-win32/winlaunch.bat
# 7z will give better results, but "meh-oh-well-for-now"
zip -r gui-build-win32.zip gui-build-win32/
