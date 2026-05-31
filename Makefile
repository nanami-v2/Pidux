# https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

build_dir := build

.PHONY: all
all: compile-test .WAIT package-test

.PHONY: compile-test
compile-test:
	cmake \
		-S tests/compile_test \
		-B $(build_dir)/compile_test \
		-DCMAKE_TOOLCHAIN_FILE="$(VCPKG_ROOT)\scripts\buildsystems\vcpkg.cmake"
	cmake --build $(build_dir)/compile_test

.PHONY: package-test
package-test:
	cmake \
		-S tests/package_test \
		-B $(build_dir)/package_test \
		-DCMAKE_TOOLCHAIN_FILE="$(VCPKG_ROOT)\scripts\buildsystems\vcpkg.cmake" \
		-DVCPKG_OVERLAY_PORTS="pidux-overlay-ports"
	cmake --build $(build_dir)/package_test

.PHONY: clean
clean:
	cmake -E rm -rf $(build_dir)
