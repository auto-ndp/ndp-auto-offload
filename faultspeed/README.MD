```bash
mkdir build
cd build
conan install .. --build=outdated
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```
