# https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration


.PHONY: samples
samples:
	cmake -S samples -B build-samples -DCMAKE_TOOLCHAIN_FILE="$(VCPKG_ROOT)\scripts\buildsystems\vcpkg.cmake"
	cmake --build build-samples

.PHONY: clean
clean:
	cmake -E rm -rf build-samples