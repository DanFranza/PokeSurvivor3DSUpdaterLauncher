#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build-tests
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Isource tests/storage_test.cpp source/storage.cpp -o build-tests/storage-tests
build-tests/storage-tests build-tests/sd
python3 -m unittest discover -s tests -p 'test_*.py'
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Itests/fakes -Isource tests/network_test.cpp source/network.cpp source/storage.cpp source/i18n.cpp \
    -o build-tests/network-tests
(cd build-tests && ./network-tests)
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Isource tests/update_source_test.cpp source/update_source.cpp source/storage.cpp \
    -o build-tests/update-source-tests
build-tests/update-source-tests
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Isource tests/i18n_test.cpp source/i18n.cpp source/storage.cpp \
    -o build-tests/i18n-tests
build-tests/i18n-tests
python3 tools/generate_translations.py --check
