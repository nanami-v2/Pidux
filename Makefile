# https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

.PHONY: cmake-config-and-generate
cmake-config-and-generate:
	cmake -B build \
		-DCMAKE_TOOLCHAIN_FILE="$(VCPKG_ROOT)\scripts\buildsystems\vcpkg.cmake" \
		-DBUILD_SAMPLE=ON

.PHONY: cmake-build
cmake-build:
	cmake --build build

.PHONY: clean
clean:
	cmake -E rm -rf build