
# Set NACL_SDK_ROOT to point to where the nacl_sdk installer is located.
# On a Mac, this might be /Users/mmortensen/Oct19update/nacl_sdk
# On a Linux system, it could be /home/mmortensen/nacl_sdk_zip/nacl_sdk
export NACL_SDK_ROOT=/home/mmortensen/nacl_sdk_zip/nacl_sdk
echo "Setting NACL_SDK_ROOT to $NACL_SDK_ROOT, which is based on naclsdk auto-updater"
echo
echo "Calling ./scons"

./scons $*
