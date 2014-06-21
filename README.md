bloom-wordgrep
==============
Grep large folders for words using bloom filters. This speeds up the search
massively because the bloomfilter can, even just given one word, reduce the
number of files that need to be searched by around 97-99%.

In this context, "word"
means something that matches [a-zA-Z_] but is not surrounded by one of
those characters (see `make_bloom_from_file` in `common.c` if this sounds
too complicated.)

Basic usage:

    cd <some project folder>
    
    # create the index, but skip all folders named "test" and "toolchain"
    makebloom . test toolchain
    
    # search for files that might contain the words "alert" and
    # "error" - will print false positives!
    bloomgrep . alert error | xargs grep "$1"


More practical:

    # show all lines in the whole source tree that match "alert(.*error"
    bloomgrep . alert error | xargs grep -w 'alert(.*error'


This is how I use this for the Chromium source tree:

```
$ ls
makebloom.sh  src  wordgrep.sh

$ cat makebloom.sh
#!/bin/sh
makebloom . .git out test LayoutTests ManualTests toolchain hunspell_dictionaries theme test_data PerformanceTests

$ cat wordgrep.sh 
#!/bin/sh
bloomgrep . "$1" | xargs grep "$1"

$ ./wordgrep.sh indexArrayArgument
stats: 93820 files searched, 2662 hits (2.837348%)
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:    const int indexArrayArgument = 1;
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:    if (V8Float32Array::hasInstance(info[indexArrayArgument], info.GetIsolate())) {
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:        Float32Array* array = V8Float32Array::toNative(info[indexArrayArgument]->ToObject());
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:    if (info[indexArrayArgument].IsEmpty() || !info[indexArrayArgument]->IsArray()) {
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:        exceptionState.throwTypeError(ExceptionMessages::argumentNullOrIncorrectType(indexArrayArgument + 1, "Array"));
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:    const int indexArrayArgumentIndex = 1;
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:    if (V8Int32Array::hasInstance(info[indexArrayArgumentIndex], info.GetIsolate())) {
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:        Int32Array* array = V8Int32Array::toNative(info[indexArrayArgumentIndex]->ToObject());
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:    if (info[indexArrayArgumentIndex].IsEmpty() || !info[indexArrayArgumentIndex]->IsArray()) {
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:        exceptionState.throwTypeError(ExceptionMessages::argumentNullOrIncorrectType(indexArrayArgumentIndex + 1, "Array"));
./src/third_party/WebKit/Source/bindings/v8/custom/V8WebGLRenderingContextCustom.cpp:    v8::Handle<v8::Array> array = v8::Local<v8::Array>::Cast(info[indexArrayArgumentIndex]);
```

Note all the exclusions - if you e.g. don't exclude `.git`, you'll
experience a significant slowdown because all the git packfiles will
trigger false positive matches because they're so big - this is caused
by the bloomfilters having a fixed per-file length.

With this, grepping the whole Chromium source tree just takes 4 seconds on my PC (with warm caches):

```
$ time ./wordgrep.sh indexArrayArgument
stats: 93820 files searched, 2662 hits (2.837348%)
[...]
real    0m3.516s
user    0m0.292s
sys     0m0.406s

```
