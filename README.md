### Build
```
sudo apt-get install libgtest-dev

pushd ./binaryninja-api && make
popd

pushd ./src
make test
popd
```
