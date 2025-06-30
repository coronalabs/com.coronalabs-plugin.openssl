
This libraries are built with https://github.com/akontsevich/openssl-android-build.git
using script `build-android-clang.sh` with added line:
```sh
build_android_clang "arm64-v8a"   "arm64" "android-21" "aarch64-linux-android" "linux-generic64 -DB_ENDIAN"
```
and this openssl https://www.openssl.org/source/openssl-1.0.2s.tar.gz