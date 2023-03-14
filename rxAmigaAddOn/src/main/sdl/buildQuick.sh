#!/bin/sh

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4b
# Set environment to CrystaX NDK with RTTI and exceptions instead of original NDK
# export PATH=$PATH:~/src/endless_space/android-ndk-r4-crystax/ndk-build
NDKBUILDPATH=$PATH
if grep "AppUseCrystaXToolchain=y" AndroidAppSettings.cfg > /dev/null ; then
	NDKBUILDPATH=`which ndk-build | sed 's@/[^/]*/ndk-build@/android-ndk-r4-crystax@'`:$PATH
fi

[ -e project/bin/lib ] || ln -s ../libs project/bin/lib

cd project && \
env PATH=$NDKBUILDPATH ndk-build -j2 V=1 

exit

cd bin && \
rm -rf DemoActivity-debug-unaligned.apk && \
cp DemoActivity.ap_ DemoActivity-debug-unaligned.apk && \
zip -1 -r DemoActivity-debug-unaligned.apk classes.dex lib && \
jarsigner -verbose -keystore ~/.android/debug.keystore -storepass android DemoActivity-debug-unaligned.apk androiddebugkey && \
rm -rf DemoActivity-debug.apk && \
zipalign 4 DemoActivity-debug-unaligned.apk DemoActivity-debug.apk && \
adb install -r DemoActivity-debug.apk

