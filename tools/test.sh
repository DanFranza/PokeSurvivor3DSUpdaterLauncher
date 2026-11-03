#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build-tests
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Isource tests/storage_test.cpp source/storage.cpp -o build-tests/storage-tests
build-tests/storage-tests build-tests/sd
python3 -m unittest discover -s tests -p 'test_*.py'
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -DNETWORK_TESTING -Isource tests/network_test.cpp source/network.cpp source/storage.cpp source/i18n.cpp \
    -lcurl -o build-tests/network-tests
python3 tests/run_network_test.py
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Isource tests/update_source_test.cpp source/update_source.cpp source/storage.cpp source/game_format.cpp \
    -o build-tests/update-source-tests
build-tests/update-source-tests
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Isource tests/i18n_test.cpp source/i18n.cpp source/storage.cpp \
    -o build-tests/i18n-tests
build-tests/i18n-tests
python3 tools/generate_translations.py --check
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -pthread -g -Isource tests/update_check_test.cpp source/update_check.cpp source/storage.cpp \
    -o build-tests/update-check-tests
build-tests/update-check-tests
"${CXX_HOST:-c++}" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -g -Itests/fakes -Isource tests/cia_test.cpp source/cia.cpp source/game_format.cpp source/storage.cpp source/i18n.cpp \
    -o build-tests/cia-tests
build-tests/cia-tests
