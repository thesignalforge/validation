#!/bin/bash
#
# Build script for Signalforge Validation PHP Extension
# Supports building for multiple PHP versions
#
# Usage:
#   ./build.sh              # Build for default PHP version
#   ./build.sh 8.4          # Build for PHP 8.4
#   ./build.sh 8.5          # Build for PHP 8.5
#   ./build.sh clean        # Clean build artifacts
#   ./build.sh test [ver]   # Run tests
#

set -e

PHP_VERSION="${1:-}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info() { echo -e "${GREEN}[INFO]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

# Clean command
if [ "$PHP_VERSION" = "clean" ]; then
    info "Cleaning build artifacts..."
    if [ -f Makefile ]; then
        make clean 2>/dev/null || true
    fi
    rm -rf autom4te.cache build modules .libs
    rm -f config.guess config.h config.h.in config.log config.nice config.status
    rm -f config.sub configure configure.ac install-sh libtool ltmain.sh
    rm -f Makefile Makefile.fragments Makefile.global Makefile.objects
    rm -f missing mkinstalldirs run-tests.php acinclude.m4 aclocal.m4
    find . -name "*.lo" -o -name "*.la" -o -name "*.o" -o -name "*.dep" | xargs rm -f 2>/dev/null || true
    find . -name ".libs" -type d | xargs rm -rf 2>/dev/null || true
    info "Clean complete"
    exit 0
fi

# Test command
if [ "$PHP_VERSION" = "test" ]; then
    PHP_VER="${2:-}"
    if [ -n "$PHP_VER" ]; then
        PHP_BIN="/usr/bin/php${PHP_VER}"
    else
        PHP_BIN="php"
    fi

    if [ ! -x "$PHP_BIN" ]; then
        error "PHP binary not found: $PHP_BIN"
    fi

    EXT_DIR="$SCRIPT_DIR/modules"
    if [ ! -f "$EXT_DIR/signalforge_validation.so" ]; then
        error "Extension not built. Run ./build.sh first"
    fi

    info "Running tests with $PHP_BIN..."
    TEST_PHP_EXECUTABLE="$PHP_BIN" "$PHP_BIN" run-tests.php \
        -d extension="$EXT_DIR/signalforge_validation.so" \
        tests/
    exit 0
fi

# Determine PHP version and binaries
if [ -n "$PHP_VERSION" ]; then
    PHPIZE="/usr/bin/phpize${PHP_VERSION}"
    PHP_CONFIG="/usr/bin/php-config${PHP_VERSION}"
    PHP_BIN="/usr/bin/php${PHP_VERSION}"
else
    PHPIZE="phpize"
    PHP_CONFIG="php-config"
    PHP_BIN="php"
fi

# Verify binaries exist
if [ ! -x "$(which $PHPIZE 2>/dev/null)" ]; then
    error "phpize not found: $PHPIZE
Install php${PHP_VERSION}-dev package:
  sudo apt-get install php${PHP_VERSION}-dev"
fi

if [ ! -x "$(which $PHP_CONFIG 2>/dev/null)" ]; then
    error "php-config not found: $PHP_CONFIG"
fi

if [ ! -x "$(which $PHP_BIN 2>/dev/null)" ]; then
    warn "PHP binary not found: $PHP_BIN (needed for testing)"
fi

# Show version info
info "Building for PHP $($PHP_CONFIG --version)"
info "  phpize: $PHPIZE"
info "  php-config: $PHP_CONFIG"
info "  Extension dir: $($PHP_CONFIG --extension-dir)"

# Clean previous build
if [ -f Makefile ]; then
    info "Cleaning previous build..."
    make clean 2>/dev/null || true
fi

# Run phpize
info "Running phpize..."
$PHPIZE --clean 2>/dev/null || true
$PHPIZE

# Configure
info "Configuring..."
./configure --with-php-config="$PHP_CONFIG" --enable-signalforge-validation

# Build
info "Building..."
make -j$(nproc)

# Show result
if [ -f modules/signalforge_validation.so ]; then
    info "Build successful!"
    echo ""
    echo "Extension built: $SCRIPT_DIR/modules/signalforge_validation.so"
    echo ""
    echo "To test:"
    echo "  ./build.sh test${PHP_VERSION:+ $PHP_VERSION}"
    echo ""
    echo "To install system-wide:"
    echo "  sudo make install"
    echo "  echo 'extension=signalforge_validation.so' | sudo tee $($PHP_CONFIG --ini-dir)/signalforge_validation.ini"
    echo ""
    echo "To use without installing:"
    echo "  $PHP_BIN -d extension=$SCRIPT_DIR/modules/signalforge_validation.so your_script.php"
else
    error "Build failed - extension not found"
fi
