bloom-wordgrep
==============
Grep large folders for words using bloom filters. This speeds up the search
massively because the bloomfilter can, even just given one word, reduce the
number of files that need to be searched by around 97-99%.

In this context, "word"
means something that matches `[a-zA-Z_]` but is not surrounded by one of
those characters (see `make_bloom_from_file` in `common.c` if this sounds
too complicated.)

Compile with `compile.sh`, then link/copy the binaries and `wordgrep.sh` into
your path.

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

    # simple wrapper: find all occurences of the word `indexArrayArgument` in the source
    wordgrep.sh indexArrayArgument


This is how I use this for the Chromium source tree:

```
$ ls
makebloom.sh  src  wordgrep.sh

$ cat makebloom.sh
#!/bin/sh
makebloom . .git out test LayoutTests ManualTests toolchain hunspell_dictionaries theme test_data PerformanceTests

$ wordgrep.sh indexArrayArgument
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

All the following timings were measured with warm caches.

With this, grepping the whole Chromium source tree just takes 4 seconds on my PC:

```
$ time wordgrep.sh indexArrayArgument
stats: 93820 files searched, 2662 hits (2.837348%)
[...]
real    0m3.516s
user    0m0.292s
sys     0m0.406s

```

Compare that to a traditional recursive grep:

```
$ time grep -RF --exclude-dir .git --exclude-dir out --exclude-dir test --exclude-dir LayoutTests --exclude-dir ManualTests --exclude-dir toolchain --exclude-dir hunspell_dictionaries --exclude-dir theme --exc$

real    1m28.150s
user    0m1.870s
sys     0m10.897s
```

But how long does it take to initially prepare the bloom filters? To "generate the index"? Just a bit more than grepping through the code:

```
$ time ./makebloom.sh >/dev/null 2>&1

real    2m8.220s
user    0m39.973s
sys     0m12.849s
```
