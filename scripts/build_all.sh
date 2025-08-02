#!/bin/bash
# scripts/build_all.sh - CFS MM Fuzzer 전체 빌드 스크립트

set -e

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 설정 변수
BUILD_TYPE="${BUILD_TYPE:-Release}"
CC="${CC:-clang}"
ENABLE_SANITIZERS="${ENABLE_SANITIZERS:-true}"
ENABLE_COVERAGE="${ENABLE_COVERAGE:-false}"
PARALLEL_JOBS="${PARALLEL_JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# 로그 함수들
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# 빌드 통계
BUILD_STATS_FILE="build_stats.log"
START_TIME=$(date +%s)

record_build_stat() {
    local component="$1"
    local status="$2"
    local duration="$3"
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $component: $status (${duration}s)" >> "$BUILD_STATS_FILE"
}

# 컴파일러 확인
check_compiler() {
    log_info "Checking compiler: $CC"
    
    if ! command -v "$CC" >/dev/null 2>&1; then
        log_error "Compiler '$CC' not found!"
        exit 1
    fi
    
    local version=$($CC --version | head -n1)
    log_info "Using: $version"
}

# 빌드 환경 설정
setup_build_env() {
    log_info "Setting up build environment..."
    
    # 빌드 디렉토리 생성
    mkdir -p build/{debug,release}
    mkdir -p bin/{debug,release}
    
    # 환경 변수 설정
    export BUILD_TYPE
    export CC
    export ENABLE_SANITIZERS
    export ENABLE_COVERAGE
    
    log_success "Build environment configured"
}

# 공통 라이브러리 빌드
build_common_libs() {
    log_info "Building common libraries..."
    local start_time=$(date +%s)
    
    cd src/common
    
    # 구문 검사
    $CC -Wall -Wextra -std=c99 -fsyntax-only utils.c
    
    cd - >/dev/null
    
    local duration=$(($(date +%s) - start_time))
    record_build_stat "common_libs" "SUCCESS" "$duration"
    log_success "Common libraries validated (${duration}s)"
}

# MM Load 하니스 빌드
build_mm_load() {
    log_info "Building MM Load harness..."
    local start_time=$(date +%s)
    
    cd src/harnesses/mm_load
    
    # 병렬 빌드 실행
    make -j"$PARALLEL_JOBS" all \
        BUILD_TYPE="$BUILD_TYPE" \
        CC="$CC" \
        ENABLE_SANITIZERS="$ENABLE_SANITIZERS" \
        ENABLE_COVERAGE="$ENABLE_COVERAGE"
    
    # AFL++ 버전도 빌드 (가능한 경우)
    make afl \
        BUILD_TYPE="$BUILD_TYPE" \
        CC="$CC" \
        ENABLE_SANITIZERS="$ENABLE_SANITIZERS" \
        ENABLE_COVERAGE="$ENABLE_COVERAGE" || log_warning "AFL++ build skipped (not available)"
    
    cd - >/dev/null
    
    local duration=$(($(date +%s) - start_time))
    record_build_stat "mm_load" "SUCCESS" "$duration"
    log_success "MM Load harness built (${duration}s)"
}

# MM Dump 하니스 빌드 (향후 확장용)
build_mm_dump() {
    if [ -f "src/harnesses/mm_dump/Makefile" ]; then
        log_info "Building MM Dump harness..."
        local start_time=$(date +%s)
        
        cd src/harnesses/mm_dump
        make -j"$PARALLEL_JOBS" all \
            BUILD_TYPE="$BUILD_TYPE" \
            CC="$CC" \
            ENABLE_SANITIZERS="$ENABLE_SANITIZERS" \
            ENABLE_COVERAGE="$ENABLE_COVERAGE"
        cd - >/dev/null
        
        local duration=$(($(date +%s) - start_time))
        record_build_stat "mm_dump" "SUCCESS" "$duration"
        log_success "MM Dump harness built (${duration}s)"
    else
        log_warning "MM Dump harness not implemented yet, skipping..."
    fi
}

# 다른 MM 하니스들 빌드 (향후 확장)
build_other_harnesses() {
    local harnesses=("mm_peek" "mm_fill")
    
    for harness in "${harnesses[@]}"; do
        if [ -f "src/harnesses/$harness/Makefile" ]; then
            log_info "Building $harness harness..."
            local start_time=$(date +%s)
            
            cd "src/harnesses/$harness"
            make -j"$PARALLEL_JOBS" all \
                BUILD_TYPE="$BUILD_TYPE" \
                CC="$CC" \
                ENABLE_SANITIZERS="$ENABLE_SANITIZERS" \
                ENABLE_COVERAGE="$ENABLE_COVERAGE"
            cd - >/dev/null
            
            local duration=$(($(date +%s) - start_time))
            record_build_stat "$harness" "SUCCESS" "$duration"
            log_success "$harness harness built (${duration}s)"
        else
            log_warning "$harness harness not implemented yet, skipping..."
        fi
    done
}

# 단위 테스트 빌드
build_unit_tests() {
    if [ -d "tests/unit" ] && [ -f "tests/unit/Makefile" ]; then
        log_info "Building unit tests..."
        local start_time=$(date +%s)
        
        cd tests/unit
        make -j"$PARALLEL_JOBS" all \
            BUILD_TYPE="$BUILD_TYPE" \
            CC="$CC" \
            ENABLE_SANITIZERS="$ENABLE_SANITIZERS" \
            ENABLE_COVERAGE="$ENABLE_COVERAGE"
        cd - >/dev/null
        
        local duration=$(($(date +%s) - start_time))
        record_build_stat "unit_tests" "SUCCESS" "$duration"
        log_success "Unit tests built (${duration}s)"
    else
        log_warning "Unit tests not implemented yet, skipping..."
    fi
}

# 통합 테스트 빌드
build_integration_tests() {
    if [ -d "tests/integration" ] && [ -f "tests/integration/Makefile" ]; then
        log_info "Building integration tests..."
        local start_time=$(date +%s)
        
        cd tests/integration
        make -j"$PARALLEL_JOBS" all \
            BUILD_TYPE="$BUILD_TYPE" \
            CC="$CC" \
            ENABLE_SANITIZERS="$ENABLE_SANITIZERS" \
            ENABLE_COVERAGE="$ENABLE_COVERAGE"
        cd - >/dev/null
        
        local duration=$(($(date +%s) - start_time))
        record_build_stat "integration_tests" "SUCCESS" "$duration"
        log_success "Integration tests built (${duration}s)"
    else
        log_warning "Integration tests not implemented yet, skipping..."
    fi
}

# 빌드 검증
verify_builds() {
    log_info "Verifying builds..."
    
    local build_dir="bin/${BUILD_TYPE,,}"  # 소문자로 변환
    local required_binaries=(
        "$build_dir/mm_load_fuzzer"
        "$build_dir/mm_load_test"
    )
    
    local missing_count=0
    for binary in "${required_binaries[@]}"; do
        if [ ! -f "$binary" ]; then
            log_error "Missing binary: $binary"
            ((missing_count++))
        else
            # 실행 권한 확인
            if [ ! -x "$binary" ]; then
                log_error "Binary not executable: $binary"
                ((missing_count++))
            else
                log_info "✓ $binary"
            fi
        fi
    done
    
    if [ $missing_count -eq 0 ]; then
        log_success "All required binaries built successfully"
        return 0
    else
        log_error "$missing_count binaries missing or not executable"
        return 1
    fi
}

# 빌드 요약 출력
print_build_summary() {
    local total_duration=$(($(date +%s) - START_TIME))
    
    echo
    log_info "=== Build Summary ==="
    log_info "Build Type: $BUILD_TYPE"
    log_info "Compiler: $CC"
    log_info "Sanitizers: $ENABLE_SANITIZERS"
    log_info "Coverage: $ENABLE_COVERAGE"
    log_info "Parallel Jobs: $PARALLEL_JOBS"
    log_info "Total Duration: ${total_duration}s"
    
    if [ -f "$BUILD_STATS_FILE" ]; then
        echo
        log_info "Component Build Times:"
        tail -10 "$BUILD_STATS_FILE" | while IFS= read -r line; do
            echo "  $line"
        done
    fi
    
    echo
    local build_dir="bin/${BUILD_TYPE,,}"
    log_info "Built binaries in $build_dir:"
    if [ -d "$build_dir" ]; then
        find "$build_dir" -type f -executable 2>/dev/null | while IFS= read -r binary; do
            local size=$(du -h "$binary" 2>/dev/null | cut -f1 || echo "?")
            echo "  $(basename "$binary") ($size)"
        done
    fi
}

# 시드 파일 생성
create_seeds() {
    log_info "Creating seed files..."
    
    cd src/harnesses/mm_load
    make create-seeds
    cd - >/dev/null
    
    log_success "Seed files created"
}

# 도움말
show_help() {
    cat << EOF
CFS MM Fuzzer 빌드 스크립트

사용법: $0 [옵션]

환경 변수:
    BUILD_TYPE          Debug 또는 Release (기본: Release)
    CC                  사용할 컴파일러 (기본: clang)
    ENABLE_SANITIZERS   Sanitizer 활성화 (기본: true)
    ENABLE_COVERAGE     Coverage 활성화 (기본: false)
    PARALLEL_JOBS       병렬 작업 수 (기본: CPU 코어 수)

옵션:
    -h, --help          이 도움말 표시
    --debug             Debug 빌드 강제
    --release           Release 빌드 강제
    --no-sanitizers     Sanitizer 비활성화
    --coverage          Coverage 활성화
    --verify-only       빌드 검증만 수행
    --clean             빌드 전 정리
    --create-seeds      시드 파일 생성
    --cfs-mode          CFS 통합 모드
    --target TARGET     특정 하니스만 빌드 (mm_load, mm_dump 등)

예제:
    $0                                    # 기본 빌드
    BUILD_TYPE=Debug $0 --coverage        # Debug + Coverage 빌드
    CC=gcc $0 --no-sanitizers            # GCC로 Sanitizer 없이 빌드
    $0 --target mm_load                   # MM Load 하니스만 빌드
EOF
}

# 정리 함수
clean_build() {
    log_info "Cleaning build artifacts..."
    
    rm -rf build/
    rm -rf bin/
    find . -name "*.o" -delete 2>/dev/null || true
    find . -name "*.a" -delete 2>/dev/null || true
    find . -name "*.gcda" -delete 2>/dev/null || true
    find . -name "*.gcno" -delete 2>/dev/null || true
    find . -name "*.info" -delete 2>/dev/null || true
    rm -f "$BUILD_STATS_FILE"
    
    # 각 하니스별 정리
    for harness_dir in src/harnesses/*/; do
        if [ -f "$harness_dir/Makefile" ]; then
            cd "$harness_dir"
            make clean 2>/dev/null || true
            cd - >/dev/null
        fi
    done
    
    log_success "Build artifacts cleaned"
}

# CFS 통합 모드 설정
setup_cfs_mode() {
    log_info "Setting up CFS integration mode..."
    
    if [ -n "$CFS_ROOT" ]; then
        export CFLAGS="$CFLAGS -DCFS_ENVIRONMENT=1 -I$CFS_ROOT/inc"
        export CXXFLAGS="$CXXFLAGS -DCFS_ENVIRONMENT=1 -I$CFS_ROOT/inc"
        log_info "CFS_ROOT detected: $CFS_ROOT"
    else
        log_warning "CFS_ROOT not set, using mock environment"
    fi
}

# 메인 함수
main() {
    local verify_only=false
    local clean_first=false
    local create_seeds_only=false
    local cfs_mode=false
    local target=""
    
    # 명령행 인수 파싱
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            --debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            --release)
                BUILD_TYPE="Release"
                shift
                ;;
            --no-sanitizers)
                ENABLE_SANITIZERS="false"
                shift
                ;;
            --coverage)
                ENABLE_COVERAGE="true"
                shift
                ;;
            --verify-only)
                verify_only=true
                shift
                ;;
            --clean)
                clean_first=true
                shift
                ;;
            --create-seeds)
                create_seeds_only=true
                shift
                ;;
            --cfs-mode)
                cfs_mode=true
                shift
                ;;
            --target)
                target="$2"
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    log_info "Starting CFS MM Fuzzer build process..."
    
    if [ "$clean_first" = "true" ]; then
        clean_build
    fi
    
    if [ "$create_seeds_only" = "true" ]; then
        create_seeds
        exit 0
    fi
    
    if [ "$verify_only" = "true" ]; then
        setup_build_env
        verify_builds
        exit $?
    fi
    
    check_compiler
    setup_build_env
    
    if [ "$cfs_mode" = "true" ]; then
        setup_cfs_mode
    fi
    
    # 빌드 순서 (의존성 고려)
    build_common_libs
    
    if [ -n "$target" ]; then
        # 특정 타겟만 빌드
        case "$target" in
            mm_load)
                build_mm_load
                ;;
            mm_dump)
                build_mm_dump
                ;;
            mm_peek|mm_fill)
                build_other_harnesses
                ;;
            *)
                log_error "Unknown target: $target"
                exit 1
                ;;
        esac
    else
        # 전체 빌드
        build_mm_load
        build_mm_dump
        build_other_harnesses
        build_unit_tests
        build_integration_tests
    fi
    
    if verify_builds; then
        print_build_summary
        log_success "Build completed successfully!"
        
        # 다음 단계 안내
        echo
        log_info "Next steps:"
        log_info "  1. Run tests: ./bin/${BUILD_TYPE,,}/mm_load_test"
        log_info "  2. Create seeds: ./scripts/build_all.sh --create-seeds"
        log_info "  3. Start fuzzing: ./scripts/run_fuzzing.sh"
    else
        log_error "Build verification failed!"
        exit 1
    fi
}

# 에러 핸들링
trap 'log_error "Build failed at line $LINENO"' ERR

# 스크립트 실행
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi