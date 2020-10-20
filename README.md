### BLLVM
#### Build

```bash
bash
sudo apt-get install libgtest-dev

# Build BinaryNinja-api
pushd ./binaryninja-api && make
popd

pushd ./src
make test && make
popd
```

