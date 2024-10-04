# @file PKGBUILD 
# @brief The package script
# @author Erick Carrillo.
# @copyright Copyright (C) 2023, Erick Alejandro Carrillo López, All right reserved.
# @license This project is released under the MIT License 

# This is an example PKGBUILD file. Use this as a start to creating your own,
# and remove these comments. For more information, see 'man PKGBUILD'.
# NOTE: Please fill out the license field for your package! If it is unknown,
# then please put 'unknown'.

# Maintainer: alecksandr <sansepiol26@gmail.com>
pkgname=mem-c
pkgver=1.0.0
pkgrel=1
epoch=
pkgdesc="mem-c is a simple memory allocator using a heap data structure with the mmap Linux syscall for dynamic memory management. It has a worst-case search time of O(n * log n) and averages O(log n). The allocator supports memory pagination, chunk merging, and plans for advanced features like garbage collection and arenas."
arch=(x86_64)
url="https://github.com/alecksandr26/mem-c/archive/refs/tags/v$pkgver.tar.gz"
license=('MIT License')
depends=()
makedepends=(gcc git make binutils coreutils)
optdepends=()
source=("$pkgname-$pkgver.tar.gz::$url")
md5sums=('SKIP')
basedir=$(pwd)

# Compile the source code 
build () {
    if [[ "${basedir##*/}" != $pkgname ]]; then
	echo "[1m[32m==>[0m[1m Compiling package..."
        tar -xf "$basedir/$pkgname-$pkgver.tar.gz"
        cd "$srcdir/$pkgname-$pkgver"
        make compile
    else
        echo "[1m[32m==>[0m[1m Compiling package locally..."
	echo "[1m[32m==>[0m[1m Trying to find the local source code path..."
	if [ -d "../../$pkgname" ]; then
	    echo "[1m[32m==>[0m[1m Source code $pkgname found..."
	    cd ../../$pkgname
	    echo "[1m[32m==>[0m[1m Compiling package..."
	    make compile -B
	    mkdir -p $srcdir/$pkgname-$pkgver
	    mkdir -p $srcdir/$pkgname-$pkgver/include
	    mkdir -p $srcdir/$pkgname-$pkgver/lib
	    cp -r include/* $srcdir/$pkgname-$pkgver/include
	    cp -r build/lib/*.so $srcdir/$pkgname-$pkgver/lib
	else
	    echo "[1m[32m==>[0m[1m Error source code $pkgname not found..."
	    exit 1
	fi
    fi
}

# Set the compiled files to create the package
# in this specific order to be able to be installed
package() {
    cd $srcdir/$pkgname-$pkgver
    mkdir -p $pkgdir/usr
    mkdir -p $pkgdir/usr/include
    mkdir -p $pkgdir/usr/lib
    cp -r include/* $pkgdir/usr/include
    cp -r lib/*.so $pkgdir/usr/lib
}
