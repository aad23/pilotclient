win32-msvc* {
    win32:contains(QMAKE_TARGET.arch, x86_64) {
        WORD_SIZE = 64
    }
    else {
        WORD_SIZE = 32
    }
}
win32-g++ {
    WIN_FIND = $$(SYSTEMROOT)\system32\find
    MINGW64 = $$system($$QMAKE_CXX -Q --help=target | $$WIN_FIND \"-m64\")
    contains(MINGW64,[enabled]) {
        WORD_SIZE = 64
    }
    else {
        WORD_SIZE = 32
    }
}
linux-g++ {
    GCC64 = $$system($$QMAKE_CXX -Q --help=target | grep m64)
    contains(GCC64,[enabled]) {
        WORD_SIZE = 64
    }
    else {
        WORD_SIZE = 32
    }
}
linux-g++-32 {
    WORD_SIZE = 32
}
linux-g++-64 {
    WORD_SIZE = 64
}
macx-clang {
    # TODO
    WORD_SIZE = 64
}
