pkgname=matrix-app
pkgver=1.0.13
pkgrel=1
pkgdesc="Video processing with Matrix effect"
arch=('x86_64')
url="https://github.com/Bit-Scripts/Matrix_CPP"
license=('MIT')
depends=('qt6-base' 'opencv' 'ffmpeg' 'v4l2loopback-dkms' 'v4l2loopback-utils')
makedepends=('cmake')
source=("v${pkgver}.tar.gz::https://github.com/Bit-Scripts/Matrix_CPP/archive/refs/tags/v${pkgver}.tar.gz")
sha256sums=('3749c4694908ceca997a460943ae3551f59cc9e87fb8b63ceed6ea08019b01e5')

build() {
  cd "${srcdir}/v${pkgver}"
  mkdir -p build
  cd build
  cmake ..
  make
}

package() {
  cd "${srcdir}/v${pkgver}/build"
  make DESTDIR="${pkgdir}/" install
}
