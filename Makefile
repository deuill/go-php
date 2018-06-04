# Package options.
NAME        := go-php
DESCRIPTION := PHP bindings for the Go programming language
IMPORT_PATH := github.com/deuill/$(NAME)
VERSION     := $(shell git describe --tags --always --dirty="-dev")

# Generic build options.
PHP_VERSION    := 7.0.30
STATIC         := false
DOCKER_IMAGE   := deuill/$(NAME):$(PHP_VERSION)

# Go build options.
GO   := go
TAGS := -tags 'php$(word 1,$(subst ., ,$(PHP_VERSION))) $(if $(findstring true,$(STATIC)),static)'

# Install options.
PREFIX := /usr

# Default Makefile options.
VERBOSE :=

# Variables to pass down to sub-invocations of 'make'.
MAKE_OPTIONS := PHP_VERSION=$(PHP_VERSION) GO=$(GO) PREFIX=$(PREFIX) VERBOSE=$(VERBOSE) STATIC=$(STATIC)

## Build binary distribution for library.
build: .build/env/GOPATH/.ok
	@echo "Building '$(NAME)'..."
	$Q $(GO) install $(if $(VERBOSE),-v) $(TAGS) $(IMPORT_PATH)

## Run test for all local packages or specified PACKAGE.
test: .build/env/GOPATH/.ok
	@echo "Running tests for '$(NAME)'..."
	$Q $(GO) test -race $(if $(VERBOSE),-v) $(TAGS) $(if $(PACKAGE),$(PACKAGE),$(PACKAGES))
	@echo "Running 'vet' for '$(NAME)'..."
	$Q $(GO) vet $(if $(VERBOSE),-v) $(TAGS) $(if $(PACKAGE),$(PACKAGE),$(PACKAGES))

## Create test coverage report for all local packages or specified PACKAGE.
cover: .build/env/GOPATH/.ok
	@echo "Creating code coverage report for '$(NAME)'..."
	$Q rm -Rf .build/tmp && mkdir -p .build/tmp
	$Q for pkg in $(if $(PACKAGE),$(PACKAGE),$(PACKAGES)); do                                    \
	       name=`echo $$pkg.cover | tr '/' '.'`;                                                 \
	       imports=`go list -f '{{ join .Imports " " }}' $$pkg`;                                 \
	       coverpkg=`echo "$$imports $(PACKAGES)" | tr ' ' '\n' | sort | uniq -d | tr '\n' ','`; \
	       $(GO) test $(if $(VERBOSE),-v) $(TAGS) -coverpkg $$coverpkg$$pkg -coverprofile .build/tmp/$$name $$pkg; done
	$Q awk "$$COVERAGE_MERGE" .build/tmp/*.cover > .build/tmp/cover.merged
	$Q $(GO) tool cover -html .build/tmp/cover.merged -o .build/tmp/coverage.html
	@echo "Coverage report written to '.build/tmp/coverage.html'"
	@echo "Total coverage for '$(NAME)':"
	$Q $(GO) tool cover -func .build/tmp/cover.merged

## Remove temporary files and packages required for build.
clean:
	@echo "Cleaning '$(NAME)'..."
	$Q $(GO) clean
	$Q rm -Rf .build

## Show usage information for this Makefile.
help:
	@printf "$(BOLD)$(DESCRIPTION)$(RESET)\n\n"
	@printf "This Makefile contains tasks for processing auxiliary actions, such as\n"
	@printf "building binaries, packages, or running tests against the test suite.\n\n"
	@printf "$(UNDERLINE)Available Tasks$(RESET)\n\n"
	@awk -F                                                                        \
	    ':|##' '/^##/ {c=$$2; getline; printf "$(BLUE)%10s$(RESET) %s\n", $$1, c}' \
	    $(MAKEFILE_LIST)
	@printf "\n"

.PHONY: build test cover clean

.DEFAULT:
	$Q $(MAKE) -s -f $(MAKEFILE) help

# Pull or build Docker image for PHP version specified.
docker-image:
	$Q docker image pull $(DOCKER_IMAGE) ||                \
	   docker build --build-arg=PHP_VERSION=$(PHP_VERSION) --build-arg=STATIC=$(STATIC) \
	                -t $(DOCKER_IMAGE) -f Dockerfile .     \

# Run Make target in Docker container. For instance, to run 'test', call as 'docker-test'.
docker-%: docker-image
	$Q docker run --rm -e GOPATH="/tmp/go"                                  \
	              -v "$(CURDIR):/tmp/go/src/$(IMPORT_PATH)" $(DOCKER_IMAGE) \
	                 "$(MAKE) -C /tmp/go/src/$(IMPORT_PATH) $(word 2,$(subst -, ,$@)) $(MAKE_OPTIONS)"

.build/env/GOPATH/.ok:
	$Q mkdir -p "$(dir .build/env/GOPATH/src/$(IMPORT_PATH))" && touch $@
	$Q ln -s ../../../../../.. ".build/env/GOPATH/src/$(IMPORT_PATH)"

MAKEFILE := $(lastword $(MAKEFILE_LIST))
Q := $(if $(VERBOSE),,@)

PACKAGES = $(shell (                                                    \
	cd $(CURDIR)/.build/env/GOPATH/src/$(IMPORT_PATH) &&                \
	GOPATH=$(CURDIR)/.build/env/GOPATH go list ./... | grep -v "vendor" \
))

export GOPATH := $(CURDIR)/.build/env/GOPATH

BOLD      = \033[1m
UNDERLINE = \033[4m
BLUE      = \033[36m
RESET     = \033[0m

define COVERAGE_MERGE
/^mode: (set|count|atomic)/ {
	if ($$2 == "set") mode = "set"
	next
}
/^mode: / {
	printf "Unknown mode '%s' in %s, line %d", $$2, FILENAME, FNR | "cat >&2"
	exit 1
}
{
	val = $$NF; $$NF = ""
	blocks[$$0] += val
}
END {
	printf "mode: %s\n", (mode == "set") ? "set" : "count"
	for (b in blocks) {
		printf "%s%d\n", b, (mode == "set" && blocks[b] > 1) ? 1 : blocks[b]
	}
}
endef
export COVERAGE_MERGE
