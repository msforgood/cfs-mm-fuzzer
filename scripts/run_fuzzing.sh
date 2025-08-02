#!/bin/bash
# scripts/run_fuzzing.sh - CFS MM Fuzzer 실행 스크립트

set -e

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 기본 설정
DEFAULT_TARGET="mm_load"
DEFAULT_FUZZER="libfuzzer"
DEFAULT_TIME=300
DEFAULT_MEMORY=1024
DEFAULT_JOBS=1

# 설정 변수
TARGET="${TARGET:-$DEFAULT_TARGET}"
FUZZER="${FUZZER:-$DEFAULT_FUZZER}"
TIME_LIMIT="${TIME_LIMIT:-$DEFAULT_TIME}"
MEMORY_LIMIT="${MEMORY_LIMIT:-$DEFAULT_MEMORY}"
PARALLEL_JOBS="${PARALLEL_JOBS:-$DEFAULT_JOBS}"
BUILD_TYPE="${BUILD_TYPE:-release}"

# 로그 함수들
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# 퍼징 결과 디렉토리 설정
setup_results_dir() {
    local timestamp=$(date +%Y%m%d_%H%M%S)
    RESULTS_DIR="results/${TARGET}_${FUZZER}_${timestamp}"
    mkdir -p "$RESULTS_DIR"/{crashes,hangs,queue,logs}
    
    log_info "Results directory: $RESULTS_DIR"
}

# 바이너리 경로 확인
check_binaries() {
    BIN_DIR="bin/${BUILD_TYPE}"
    
    case "$TARGET" in
        mm_load)
            case "$FUZZER" in
                libfuzzer)
                    FUZZER_BIN="$BIN_DIR/mm_load_fuzzer"
                    ;;
                afl)
                    FUZZER_BIN="$BIN_DIR/mm_load_afl"
                    ;;
                standalone)
                    FUZZER_BIN="$BIN_DIR/mm_load_test"
                    ;;
                *)
                    log_error "Unknown fuzzer: $FUZZER"
                    exit 1
                    ;;
            esac
            CORPUS_DIR="corpus/mm_load"
            ;;
        mm_dump)
            case "$FUZZER" in
                libfuzzer)
                    FUZZER_BIN="$BIN_DIR/mm_dump_fuzzer"
                    ;;
                afl)
                    FUZZER_BIN="$BIN_DIR/mm_dump_afl"
                    ;;
                standalone)
                    FUZZER_BIN="$BIN_DIR/mm_dump_test"
                    ;;
                *)
                    log_error "Unknown fuzzer: $FUZZER"
                    exit 1
                    ;;
            esac
            CORPUS_DIR="corpus/mm_dump"
            ;;
        *)
            log_error "Unknown target: $TARGET"
            exit 1
            ;;
    esac
    
    if [ ! -f "$FUZZER_BIN" ]; then
        log_error "Binary not found: $FUZZER_BIN"
        log_info "Please run: ./scripts/build_all.sh"
        exit 1
    fi
    
    if [ ! -x "$FUZZER_BIN" ]; then
        log_error "Binary not executable: $FUZZER_BIN"
        exit 1
    fi
    
    log_info "Using binary: $FUZZER_BIN"
}

# 코퍼스 디렉토리 확인/생성
setup_corpus() {
    if [ ! -d "$CORPUS_DIR" ]; then
        log_warning "Corpus directory not found: $CORPUS_DIR"
        log_info "Creating corpus directory and seed files..."
        
        mkdir -p "$CORPUS_DIR"
        
        # 타겟별 시드 생성
        case "$TARGET" in
            mm_load)
                cd src/harnesses/mm_load
                make create-seeds
                cd - >/dev/null
                ;;
            mm_dump)
                if [ -f "src/harnesses/mm_dump/Makefile" ]; then
                    cd src/harnesses/mm_dump
                    make create-seeds
                    cd - >/dev/null
                fi
                ;;
        esac
    fi
    
    local seed_count=$(find "$CORPUS_DIR" -type f | wc -l)
    log_info "Corpus directory: $CORPUS_DIR ($seed_count seeds)"
}

# LibFuzzer 실행
run_libfuzzer() {
    log_info "Starting LibFuzzer..."
    log_info "Target: $TARGET"
    log_info "Time limit: ${TIME_LIMIT}s"
    log_info "Memory limit: ${MEMORY_LIMIT}MB"
    log_info "Parallel jobs: $PARALLEL_JOBS"
    
    local fuzzer_args=(
        "$CORPUS_DIR"
        "-max_total_time=$TIME_LIMIT"
        "-rss_limit_mb=$MEMORY_LIMIT"
        "-print_final_stats=1"
        "-artifact_prefix=$RESULTS_DIR/crashes/"
        "-exact_artifact_path=$RESULTS_DIR/crashes/crash-"
    )
    
    # 병렬 실행 지원
    if [ "$PARALLEL_JOBS" -gt 1 ]; then
        fuzzer_args+=("-jobs=$PARALLEL_JOBS" "-workers=$PARALLEL_JOBS")
    fi
    
    # 딕셔너리 파일이 있으면 사용
    local dict_file="configs/${TARGET}_dict.txt"
    if [ -f "$dict_file" ]; then
        fuzzer_args+=("-dict=$dict_file")
        log_info "Using dictionary: $dict_file"
    fi
    
    # 퍼징 실행
    echo "Command: $FUZZER_BIN ${fuzzer_args[*]}" | tee "$RESULTS_DIR/logs/command.log"
    
    # 실시간 로그 저장
    {
        echo "=== LibFuzzer Session Started at $(date) ==="
        echo "Target: $TARGET"
        echo "Binary: $FUZZER_BIN"
        echo "Corpus: $CORPUS_DIR"
        echo "Time Limit: ${TIME_LIMIT}s"
        echo "Memory Limit: ${MEMORY_LIMIT}MB"
        echo "Parallel Jobs: $PARALLEL_JOBS"
        echo "Results: $RESULTS_DIR"
        echo "================================"
        echo
    } > "$RESULTS_DIR/logs/session.log"
    
    "$FUZZER_BIN" "${fuzzer_args[@]}" 2>&1 | tee -a "$RESULTS_DIR/logs/session.log"
    
    local exit_code=${PIPESTATUS[0]}
    
    {
        echo
        echo "=== LibFuzzer Session Ended at $(date) ==="
        echo "Exit Code: $exit_code"
        echo "================================"
    } >> "$RESULTS_DIR/logs/session.log"
    
    return $exit_code
}

# AFL++ 실행
run_afl() {
    log_info "Starting AFL++..."
    
    if ! command -v afl-fuzz >/dev/null 2>&1; then
        log_error "AFL++ not found. Please install AFL++."
        exit 1
    fi
    
    local afl_output="$RESULTS_DIR/afl_output"
    mkdir -p "$afl_output"
    
    local afl_args=(
        "-i" "$CORPUS_DIR"
        "-o" "$afl_output"
        "-t" "1000+"
        "-m" "$MEMORY_LIMIT"
    )
    
    # 딕셔너리 파일이 있으면 사용
    local dict_file="configs/${TARGET}_dict.txt"
    if [ -f "$dict_file" ]; then
        afl_args+=("-x" "$dict_file")
        log_info "Using dictionary: $dict_file"
    fi
    
    # 단일 퍼저 실행
    afl_args+=("-M" "fuzzer01")
    echo "Command: afl-fuzz ${afl_args[*]} -- $FUZZER_BIN" | tee "$RESULTS_DIR/logs/afl_command.log"
    
    timeout "${TIME_LIMIT}s" afl-fuzz "${afl_args[@]}" -- "$FUZZER_BIN" \
        2>&1 | tee "$RESULTS_DIR/logs/afl_session.log" || true
    
    # 결과 정리
    if [ -d "$afl_output" ]; then
        find "$afl_output" -name "crashes" -type d -exec cp -r {} "$RESULTS_DIR/" \; 2>/dev/null || true
        find "$afl_output" -name "hangs" -type d -exec cp -r {} "$RESULTS_DIR/" \; 2>/dev/null || true
    fi
}

# 독립 실행 테스트
run_standalone() {
    log_info "Running standalone test..."
    
    {
        echo "=== Standalone Test Started at $(date) ==="
        echo "Target: $TARGET"
        echo "Binary: $FUZZER_BIN"
        echo "================================"
        echo
    } > "$RESULTS_DIR/logs/standalone.log"
    
    "$FUZZER_BIN" 2>&1 | tee -a "$RESULTS_DIR/logs/standalone.log"
    local exit_code=${PIPESTATUS[0]}
    
    {
        echo
        echo "=== Standalone Test Ended at $(date) ==="
        echo "Exit Code: $exit_code"
        echo "================================"
    } >> "$RESULTS_DIR/logs/standalone.log"
    
    return $exit_code
}

# 결과 분석
analyze_results() {
    log_info "Analyzing fuzzing results..."
    
    local crash_count=$(find "$RESULTS_DIR/crashes" -type f 2>/dev/null | wc -l)
    local hang_count=$(find "$RESULTS_DIR/hangs" -type f 2>/dev/null | wc -l)
    
    {
        echo "=== Fuzzing Results Summary ==="
        echo "Session: $(basename "$RESULTS_DIR")"
        echo "Target: $TARGET"
        echo "Fuzzer: $FUZZER"
        echo "Duration: ${TIME_LIMIT}s"
        echo "Crashes Found: $crash_count"
        echo "Hangs Found: $hang_count"
        echo "Results Directory: $RESULTS_DIR"
        echo "=============================="
    } | tee "$RESULTS_DIR/summary.txt"
    
    if [ "$crash_count" -gt 0 ]; then
        log_warning "Found $crash_count crashes!"
        log_info "Crash files saved in: $RESULTS_DIR/crashes/"
        
        # 크래시 파일 최소화 (LibFuzzer만)
        if [ "$FUZZER" = "libfuzzer" ] && [ "$crash_count" -lt 10 ]; then
            log_info "Minimizing crash files..."
            for crash_file in "$RESULTS_DIR/crashes"/*; do
                if [ -f "$crash_file" ]; then
                    local minimized_file="${crash_file}.min"
                    timeout 60s "$FUZZER_BIN" -minimize_crash=1 "$crash_file" \
                        -exact_artifact_path="$minimized_file" 2>/dev/null || true
                fi
            done
        fi
    fi
    
    if [ "$hang_count" -gt 0 ]; then
        log_warning "Found $hang_count hangs!"
        log_info "Hang files saved in: $RESULTS_DIR/hangs/"
    fi
    
    if [ "$crash_count" -eq 0 ] && [ "$hang_count" -eq 0 ]; then
        log_success "No crashes or hangs found - target appears stable"
    fi
    
    # 로그 파일 압축
    if [ -d "$RESULTS_DIR/logs" ]; then
        tar -czf "$RESULTS_DIR/logs.tar.gz" -C "$RESULTS_DIR" logs
        log_info "Logs archived: $RESULTS_DIR/logs.tar.gz"
    fi
}

# 도움말
show_help() {
    cat << EOF
CFS MM Fuzzer 실행 스크립트

사용법: $0 [옵션]

옵션:
    -h, --help              이 도움말 표시
    --target TARGET         타겟 하니스 (mm_load, mm_dump, mm_peek, mm_fill)
    --fuzzer FUZZER         퍼저 (libfuzzer, afl, standalone)
    --time SECONDS          퍼징 시간 (초)
    --memory MB             메모리 제한 (MB)
    --jobs N                병렬 작업 수
    --build-type TYPE       빌드 타입 (debug, release)
    --cfs-mode              CFS 통합 모드
    --continuous            지속적 퍼징 모드

환경 변수:
    TARGET                  타겟 하니스 (기본: mm_load)
    FUZZER                  퍼저 타입 (기본: libfuzzer)
    TIME_LIMIT              퍼징 시간 (기본: 300초)
    MEMORY_LIMIT            메모리 제한 (기본: 1024MB)
    PARALLEL_JOBS           병렬 작업 수 (기본: 1)
    BUILD_TYPE              빌드 타입 (기본: release)

예제:
    $0                                           # 기본 설정으로 mm_load 퍼징
    $0 --target mm_dump --fuzzer afl            # AFL++로 mm_dump 퍼징
    $0 --time 3600 --jobs 4                     # 1시간, 4개 병렬 작업
    TARGET=mm_load FUZZER=libfuzzer $0          # 환경변수 사용
    $0 --fuzzer standalone                       # 단독 테스트 실행
    $0 --continuous --time 86400                # 24시간 지속적 퍼징

지원하는 타겟:
    mm_load     - MM LoadMemFromFileCmd 퍼징
    mm_dump     - MM DumpMemToFileCmd 퍼징 (구현 예정)
    mm_peek     - MM PeekCmd 퍼징 (구현 예정)
    mm_fill     - MM FillMemCmd 퍼징 (구현 예정)

지원하는 퍼저:
    libfuzzer   - LLVM LibFuzzer (기본)
    afl         - AFL++ 퍼저
    standalone  - 독립 실행 테스트
EOF
}

# 지속적 퍼징 모드
run_continuous() {
    log_info "Starting continuous fuzzing mode..."
    
    local session_count=1
    local start_time=$(date +%s)
    
    while true; do
        log_info "=== Fuzzing Session $session_count ==="
        
        # 새로운 결과 디렉토리 설정
        setup_results_dir
        
        # 퍼징 실행
        case "$FUZZER" in
            libfuzzer)
                run_libfuzzer || true
                ;;
            afl)
                run_afl || true
                ;;
            *)
                log_error "Continuous mode not supported for fuzzer: $FUZZER"
                exit 1
                ;;
        esac
        
        # 결과 분석
        analyze_results
        
        # 세션 완료
        local elapsed=$(($(date +%s) - start_time))
        log_info "Session $session_count completed (total elapsed: ${elapsed}s)"
        
        ((session_count++))
        
        # 잠시 대기
        sleep 10
    done
}

# 메인 함수
main() {
    local continuous_mode=false
    local cfs_mode=false
    
    # 명령행 인수 파싱
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            --target)
                TARGET="$2"
                shift 2
                ;;
            --fuzzer)
                FUZZER="$2"
                shift 2
                ;;
            --time)
                TIME_LIMIT="$2"
                shift 2
                ;;
            --memory)
                MEMORY_LIMIT="$2"
                shift 2
                ;;
            --jobs)
                PARALLEL_JOBS="$2"
                shift 2
                ;;
            --build-type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            --cfs-mode)
                cfs_mode=true
                shift
                ;;
            --continuous)
                continuous_mode=true
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    log_info "=== CFS MM Fuzzer ==="
    log_info "Target: $TARGET"
    log_info "Fuzzer: $FUZZER"
    log_info "Time Limit: ${TIME_LIMIT}s"
    log_info "Memory Limit: ${MEMORY_LIMIT}MB"
    log_info "Parallel Jobs: $PARALLEL_JOBS"
    log_info "Build Type: $BUILD_TYPE"
    
    if [ "$cfs_mode" = "true" ]; then
        log_info "CFS Integration: Enabled"
        export CFS_FUZZING_MODE=1
    fi
    
    # 사전 검사
    check_binaries
    setup_corpus
    
    if [ "$continuous_mode" = "true" ]; then
        run_continuous
    else
        setup_results_dir
        
        # 퍼저별 실행
        case "$FUZZER" in
            libfuzzer)
                run_libfuzzer
                ;;
            afl)
                run_afl
                ;;
            standalone)
                run_standalone
                ;;
            *)
                log_error "Unknown fuzzer: $FUZZER"
                exit 1
                ;;
        esac
        
        # 결과 분석
        analyze_results
        
        log_success "Fuzzing session completed!"
        log_info "Results saved in: $RESULTS_DIR"
    fi
}

# 에러 핸들링
trap 'log_error "Fuzzing failed at line $LINENO"' ERR

# 스크립트 실행
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi