### BLLVM
#### Build

```bash
bash
sudo apt-get install libgtest-dev

# Build BinaryNinja-api
pushd ./binaryninja-api
mkdir build
pushd ./build
cmake -DBN_INSTALL_DIR=/opt/binaryninja ../
make
popd
popd

pushd ./src
make test && make
popd
```

#### TODO
* BRANCH INSTRUCTION
