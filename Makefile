VERSION_FILE := version.txt

PROJECT_NAME := keepalived-exporter
PKG := "github.com/ottopia-tech/$(PROJECT_NAME)"
LINTER = golangci-lint
LINTER_VERSION = 1.50.1
CURRENT_LINTER_VERSION := $(shell golangci-lint version 2>/dev/null | awk '{ print $$4 }')

BUILD_TIME := $(shell LANG=en_US date +"%F_%T_%z")
COMMIT := $(shell git rev-parse --short HEAD)
VERSION ?= $(shell git describe --tags ${COMMIT} 2>/dev/null | cut -c2-)
VERSION := $(or $(VERSION),$(COMMIT))
LD_FLAGS ?=
LD_FLAGS += -X main.version=$(VERSION)
LD_FLAGS += -X main.commit=$(COMMIT)
LD_FLAGS += -X main.buildTime=$(BUILD_TIME)

PYTHON_VER ?= 3.9
PYTHON_COMMAND ?= python
PIP_COMMAND ?= pip
VENV_PATH ?= venv
VENV_BIN ?= $(VENV_PATH)/bin
PYPI_SERVER ?= ""
ifneq ($(PYPI_SERVER), "")
	PYPI_CMD=-i $(PYPI_SERVER)
endif



ARCH := $(shell uname -m)
ifeq ($(ARCH), x86_64)
	ARCH = amd64
endif

RELEASE_FILENAME := $(PROJECT_NAME)-$(VERSION).linux-$(ARCH)

.PHONY: all dep lint build clean format test install release

all: dep build

dep: ## Get the dependencies
	@go mod tidy

lintdeps: ## golangci-lint dependencies
ifneq ($(CURRENT_LINTER_VERSION), $(LINTER_VERSION))
	@curl -sfL https://raw.githubusercontent.com/golangci/golangci-lint/master/install.sh | sh -s -- -b $(GOPATH)/bin v$(LINTER_VERSION)
endif

lint: lintdeps ## to lint the files
	$(LINTER) run --config=.golangci-lint.yml ./...

build: $(VERSION_FILE) ## Build the binary file
	@go build -o $(PROJECT_NAME) -v -ldflags="$(LD_FLAGS)" cmd/$(PROJECT_NAME)/main.go

test:
	@go test -v -cover -race ./...

clean: ## Remove previous build and release files
	@rm -f $(PROJECT_NAME)
	@rm -f $(RELEASE_FILENAME).zip
	@rm -f $(RELEASE_FILENAME).tar.gz

release: build
	@mkdir $(RELEASE_FILENAME)
	@cp $(PROJECT_NAME) $(RELEASE_FILENAME)/
	@cp LICENSE $(RELEASE_FILENAME)/
	@zip -r $(RELEASE_FILENAME).zip $(RELEASE_FILENAME)
	@tar -czvf $(RELEASE_FILENAME).tar.gz $(RELEASE_FILENAME)
	@rm -rf $(RELEASE_FILENAME)

$(VERSION_FILE): # Creates $(VERSION_FILE) file
	@echo "GIT_BRANCH \"`git rev-parse --abbrev-ref HEAD`\"" > $(VERSION_FILE)
	@echo "COMMIT_HASH \"`git rev-parse --short HEAD`\"" >> $(VERSION_FILE)
	@echo "BUILD_DATE_UTC \"`date -u +"%F %T %Z"`\"" >> $(VERSION_FILE)

$(VENV_PATH):
	$(PYTHON_COMMAND)$(PYTHON_VER) -m venv $(VENV_PATH)
	$(VENV_BIN)/$(PIP_COMMAND) install --upgrade $(PIP_COMMAND)

$(VENV_PATH)/req-done: $(VENV_PATH) requirements.txt
	$(VENV_BIN)/$(PIP_COMMAND) install -r requirements.txt $(PYPI_CMD)
	@touch $@

install: $(VENV_PATH)/req-done ## install dependencies for production
	$(VENV_BIN)/$(PYTHON_COMMAND) -m $(PIP_COMMAND) install -e . $(PYPI_CMD)

format: ## check formatting
	$(VENV_BIN)/$(PYTHON_COMMAND) -m black --check app tests
	$(VENV_BIN)/$(PYTHON_COMMAND) -m isort --profile black -c app tests

