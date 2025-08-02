# MM Load 하니스 Makefile

CC = clang
CXX = clang++

# 기본 컴파일 플래그
CFLAGS_BASE = -Wall -Wextra -std=c99
CXXFLAGS_BASE = -Wall -Wextra -std=c++11

# 빌드 타입별 플래그
ifeq ($(BUILD_TYPE),Debug)
    CFLAGS = $(CFLAGS_BASE) -g -O0 -DCFS_DEBUG_MODE=1
    CXXFLAGS = $(CXXFLAGS_BASE) -g -O0 -DCFS_DEBUG_MODE=1
    BUILD_DIR = ../../../build/debug
    BIN_DIR = ../../../bin/debug
else
    CFLAGS = $(CFLAGS_BASE) -O2 -DNDEBUG
    CXXFLAGS = $(CXXFLAGS_BASE) -O2 -DNDEBUG
    BUILD_DIR = ../../../build/release
    BIN_DIR = ../../../bin/release
endif

# Sanitizer 플래그
ifeq ($(ENABLE_SANITIZERS),true)
    SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
    CFLAGS += $(SANITIZER_FLAGS)
    CXXFLAGS += $(SANITIZER_FLAGS)
endif

# Coverage 플래그
ifeq ($(ENABLE_COVERAGE),true)
    COVERAGE_FLAGS = --coverage
    CFLAGS += $(COVERAGE_FLAGS)
    CXXFLAGS += $(COVERAGE_FLAGS)
endif

# LibFuzzer 플래그
FUZZER_FLAGS = -fsanitize=fuzzer,address

# 소스 파일들
HARNESS_SRC = mm_load_harness.c
COMMON_DIR = ../../common
COMMON_SOURCES = $(COMMON_DIR)/utils.c

# 출력 바이너리들
FUZZER_BIN = $(BIN_DIR)/mm_load_fuzzer
STANDALONE_BIN = $(BIN_DIR)/mm_load_test
AFL_BIN = $(BIN_DIR)/mm_load_afl

# 포함 디렉토리
INCLUDES = -I$(COMMON_DIR)

# 라이브러리
LIBS = -lm

# 기본 타겟
all: directories fuzzer standalone

# 디렉토리 생성
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# LibFuzzer 빌드
fuzzer: directories
	$(CC) $(CFLAGS) $(FUZZER_FLAGS) $(INCLUDES) \
		-o $(FUZZER_BIN) \
		$(HARNESS_SRC) $(COMMON_SOURCES) $(LIBS)
	@echo "Built LibFuzzer version: $(FUZZER_BIN)"

# 독립 실행 테스트 빌드
standalone: directories
	$(CC) $(CFLAGS) -DSTANDALONE_TEST $(INCLUDES) \
		-o $(STANDALONE_BIN) \
		$(HARNESS_SRC) $(COMMON_SOURCES) $(LIBS)
	@echo "Built standalone test: $(STANDALONE_BIN)"

# AFL++ 빌드 (AFL++가 설치된 경우)
afl: directories
	@if command -v afl-clang-fast >/dev/null 2>&1; then \
		afl-clang-fast $(CFLAGS) -DSTANDALONE_TEST $(INCLUDES) \
			-o $(AFL_BIN) \
			$(HARNESS_SRC) $(COMMON_SOURCES) $(LIBS); \
		echo "Built AFL++ version: $(AFL_BIN)"; \
	else \
		echo "AFL++ not found, skipping AFL build"; \
	fi

# Coverage 빌드
coverage: directories
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -DSTANDALONE_TEST $(INCLUDES) \
		-o $(STANDALONE_BIN)_cov \
		$(HARNESS_SRC) $(COMMON_SOURCES) $(LIBS)
	@echo "Built coverage version: $(STANDALONE_BIN)_cov"

# 구문 검사만
test-syntax:
	$(CC) $(CFLAGS) $(INCLUDES) -fsyntax-only $(HARNESS_SRC)
	@echo "Syntax check passed"

# 정적 분석
static-analysis:
	@if command -v scan-build >/dev/null 2>&1; then \
		scan-build --status-bugs $(CC) $(CFLAGS) $(INCLUDES) \
			-o /dev/null $(HARNESS_SRC) $(COMMON_SOURCES); \
	else \
		echo "scan-build not found, skipping static analysis"; \
	fi

# 독립 테스트 실행
run-test: standalone
	@echo "Running standalone test..."
	$(STANDALONE_BIN)

# 퍼징 실행 (5분간)
run-fuzzer: fuzzer
	@echo "Running LibFuzzer for 5 minutes..."
	@mkdir -p ../../../corpus/mm_load
	$(FUZZER_BIN) ../../../corpus/mm_load -max_total_time=300 -print_final_stats=1

# 시드 생성
create-seeds:
	@echo "Creating seed files..."
	@mkdir -p ../../../corpus/mm_load
	# 기본 유효한 패킷
	@python3 -c "\
import struct; \
data = bytearray(200); \
data[0] = 1; \
data[1:5] = struct.pack('<I', 1024); \
data[5:9] = struct.pack('<I', 0); \
data[9] = 0; \
data[10] = 0; \
data[11] = 0; \
with open('../../../corpus/mm_load/valid_ram.bin', 'wb') as f: f.write(data)"
	
	# EEPROM 타입 패킷
	@python3 -c "\
import struct; \
data = bytearray(200); \
data[0] = 2; \
data[1:5] = struct.pack('<I', 65536); \
data[5:9] = struct.pack('<I', 0x12345678); \
data[9] = 1; \
data[10:18] = struct.pack('<Q', 0x08080000); \
data[18] = 1; \
with open('../../../corpus/mm_load/valid_eeprom.bin', 'wb') as f: f.write(data)"
	
	# 최대 크기 패킷
	@python3 -c "\
import struct; \
data = bytearray(200); \
data[0] = 1; \
data[1:5] = struct.pack('<I', 1048576); \
data[5:9] = struct.pack('<I', 0); \
data[9] = 0; \
data[10] = 0; \
data[11] = 2; \
with open('../../../corpus/mm_load/max_size.bin', 'wb') as f: f.write(data)"
	
	@echo "Seed files created in ../../../corpus/mm_load/"

# Coverage 보고서 생성
coverage-report: coverage
	@echo "Generating coverage report..."
	./$(STANDALONE_BIN)_cov
	@if command -v lcov >/dev/null 2>&1; then \
		lcov --capture --directory . --output-file mm_load_coverage.info; \
		lcov --remove mm_load_coverage.info '/usr/*' --output-file mm_load_coverage.info; \
		genhtml mm_load_coverage.info --output-directory coverage_html; \
		echo "Coverage report generated in coverage_html/"; \
	else \
		echo "lcov not found, generating simple coverage report"; \
		gcov $(HARNESS_SRC); \
	fi

# 메모리 검사 (Valgrind)
memcheck: standalone
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Running memory check with Valgrind..."; \
		valgrind --leak-check=full --error-exitcode=1 $(STANDALONE_BIN); \
	else \
		echo "Valgrind not found, skipping memory check"; \
	fi

# 성능 프로파일링
profile: standalone
	@if command -v perf >/dev/null 2>&1; then \
		echo "Running performance profiling..."; \
		perf record -g $(STANDALONE_BIN); \
		perf report; \
	else \
		echo "perf not found, skipping profiling"; \
	fi

# 디버그 정보 출력
debug-info:
	@echo "=== Build Configuration ==="
	@echo "CC: $(CC)"
	@echo "BUILD_TYPE: $(BUILD_TYPE)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "INCLUDES: $(INCLUDES)"
	@echo "BUILD_DIR: $(BUILD_DIR)"
	@echo "BIN_DIR: $(BIN_DIR)"
	@echo "=========================="

# 정리
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(FUZZER_BIN) $(STANDALONE_BIN) $(AFL_BIN)
	@rm -f $(STANDALONE_BIN)_cov
	@rm -f *.gcda *.gcno *.gcov
	@rm -f *.info
	@rm -rf coverage_html/
	@rm -f perf.data*
	@echo "Clean completed"

# 도움말
help:
	@echo "Available targets:"
	@echo "  all           - Build fuzzer and standalone versions"
	@echo "  fuzzer        - Build LibFuzzer version"
	@echo "  standalone    - Build standalone test version"
	@echo "  afl          - Build AFL++ version (if available)"
	@echo "  coverage     - Build with coverage instrumentation"
	@echo "  test-syntax  - Check syntax only"
	@echo "  static-analysis - Run static analysis (if available)"
	@echo "  run-test     - Run standalone test"
	@echo "  run-fuzzer   - Run LibFuzzer for 5 minutes"
	@echo "  create-seeds - Create initial seed files"
	@echo "  coverage-report - Generate coverage report"
	@echo "  memcheck     - Run memory check with Valgrind"
	@echo "  profile      - Run performance profiling"
	@echo "  debug-info   - Show build configuration"
	@echo "  clean        - Clean build artifacts"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Environment variables:"
	@echo "  BUILD_TYPE        - Debug or Release (default: Release)"
	@echo "  CC               - Compiler to use (default: clang)"
	@echo "  ENABLE_SANITIZERS - Enable sanitizers (default: false)"
	@echo "  ENABLE_COVERAGE  - Enable coverage (default: false)"

.PHONY: all directories fuzzer standalone afl coverage test-syntax static-analysis \
        run-test run-fuzzer create-seeds coverage-report memcheck profile \
        debug-info clean help