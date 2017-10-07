FROM debian:stable-slim

# The full PHP version to target, i.e. "7.1.10".
ARG PHP_VERSION

# Environment variables used across the build.
ENV PHP_URL="https://secure.php.net/get/php-${PHP_VERSION}.tar.xz/from/this/mirror" PHP_ASC_URL="https://secure.php.net/get/php-${PHP_VERSION}.tar.xz.asc/from/this/mirror"
ENV PHP_BASE_DIR="/tmp/php"
ENV PHP_SRC_DIR="${PHP_BASE_DIR}/src"

# Build variables.
ENV PHP_LDFLAGS="-Wl,-O1 -Wl,--hash-style=both -pie"
ENV PHP_CFLAGS="-fstack-protector-strong -fpic -fpie -O2"
ENV PHP_CPPFLAGS="${PHP_CFLAGS}"

# Fetch PHP source code. This step does not currently validate keys or checksums, as this process
# will eventually transition to using the base `php` Docker images.
ENV FETCH_DEPS="ca-certificates wget"
RUN set -xe && \
	apt-get update && apt-get install -y --no-install-recommends ${FETCH_DEPS} && \
	mkdir -p ${PHP_BASE_DIR} && cd ${PHP_BASE_DIR} && \
	wget -O php.tar.xz ${PHP_URL} && \
    apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false ${FETCH_DEPS}

# Build PHP library from source.
ENV BUILD_DEPS="build-essential file libpcre3-dev dpkg-dev libcurl4-openssl-dev libedit-dev libsqlite3-dev libssl1.0-dev libxml2-dev zlib1g-dev"
RUN set -xe && \
	apt-get update && apt-get install -y --no-install-recommends ${BUILD_DEPS} && \
    export CFLAGS="${PHP_CFLAGS}" CPPFLAGS="${PHP_CPPFLAGS}" LDFLAGS="${PHP_LDFLAGS}" && \
    arch="$(dpkg-architecture --query DEB_BUILD_GNU_TYPE)" && multiarch="$(dpkg-architecture --query DEB_BUILD_MULTIARCH)" && \
    if [ ! -d /usr/include/curl ]; \
        then ln -sT "/usr/include/$multiarch/curl" /usr/local/include/curl; \
    fi && \
    mkdir -p ${PHP_SRC_DIR} && cd ${PHP_SRC_DIR} && \
    tar -xJf ${PHP_BASE_DIR}/php.tar.xz -C . --strip-components=1 && \
    ./configure \
        --prefix=/usr --build="$arch" \
        --with-libdir="lib/$multiarch" \
        --with-pcre-regex=/usr \
        --disable-cgi --disable-fpm \
        --enable-embed --enable-ftp --enable-mbstring \
        --with-curl --with-libedit --with-openssl --with-zlib \
        && \
    make -j "$(nproc)" && \
    apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false ${BUILD_DEPS}

# Install runtime dependencies for testing, building packages etc, and clean up source.
ENV RUNTIME_DEPS="build-essential git golang curl libedit2 libssl1.0 libxml2"
ENV SOURCE_REPO="github.com/deuill/go-php"

RUN set -xe && \
	apt-get update && apt-get install -y --no-install-recommends ${RUNTIME_DEPS} && \
    cd ${PHP_SRC_DIR} && make -j "$(nproc)" PHP_SAPI=embed install-sapi install-headers && \
    cd / && rm -Rf ${PHP_BASE_DIR} ${PHP_SRC_DIR}

ENTRYPOINT ["/bin/sh", "-c"]
