FROM debian:stable-slim

ENV PHP_VERSION="5.6.31"
ENV PHP_URL="https://secure.php.net/get/php-${PHP_VERSION}.tar.xz/from/this/mirror" PHP_ASC_URL="https://secure.php.net/get/php-${PHP_VERSION}.tar.xz.asc/from/this/mirror"

ENV PHP_BASE_DIR="/tmp/php"
ENV PHP_SRC_DIR="${PHP_BASE_DIR}/src"

ENV PHP_LDFLAGS="-Wl,-O1 -Wl,--hash-style=both -pie"
ENV PHP_CFLAGS="-fstack-protector-strong -fpic -fpie -O2"
ENV PHP_CPPFLAGS="${PHP_CFLAGS}"

ENV GPG_KEYS="0BD78B5F97500D450838F95DFE857D9A90D90EC1 6E4F6AB321FDC07F2C332E3AC2BF0BC433CFC8B3"
ENV PHP_SHA256="c464af61240a9b7729fabe0314cdbdd5a000a4f0c9bd201f89f8628732fe4ae4"

ENV FETCH_DEPS="ca-certificates wget dirmngr gnupg2"

RUN set -xe && \
	apt-get update && apt-get install -y --no-install-recommends ${FETCH_DEPS} && \
	mkdir -p ${PHP_BASE_DIR} && cd ${PHP_BASE_DIR} && \
	wget -O php.tar.xz ${PHP_URL} && \
    echo "${PHP_SHA256} *php.tar.xz" | sha256sum -c - && \
    wget -O php.tar.xz.asc "${PHP_ASC_URL}" && \
    export GNUPGHOME="$(mktemp -d)" && \
    for key in ${GPG_KEYS}; do gpg --keyserver ha.pool.sks-keyservers.net --recv-keys "$key"; done && \
    gpg --batch --verify php.tar.xz.asc php.tar.xz && \
    rm -Rf ${GNUPGHOME} && \
    apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false ${FETCH_DEPS}

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

ENV RUNTIME_DEPS="build-essential git golang curl libedit2 libssl1.0 libxml2"
ENV SOURCE_REPO="github.com/deuill/go-php"

RUN set -xe && \
	apt-get update && apt-get install -y --no-install-recommends ${RUNTIME_DEPS} && \
    cd ${PHP_SRC_DIR} && make -j "$(nproc)" PHP_SAPI=embed install-sapi install-headers && \
    cd / && rm -Rf ${PHP_BASE_DIR} ${PHP_SRC_DIR}

ENTRYPOINT ["/bin/sh", "-c"]
