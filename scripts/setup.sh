#!/bin/bash
# scripts/setup.sh - CFS MM Fuzzer 환경 설정 스크립트 (간소화 버전)

set -e

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# 기본 디렉토리 생성
create_directories() {
    log_info "Creating directory structure..."
    
    local dirs=(
        "bin/debug"
        "bin/release"
        "build/debug"
        "build/release"
        "corpus/mm_load"
        "results"
        "logs"
    )
    
    for dir in "${dirs[@]}"; do
        mkdir -p "$dir"
    done
    
    log_success "Directory structure created"
}

# 스크립트 실행 권한 설정
set_permissions() {
    log_info "Setting script permissions..."
    
    find scripts/ -name "*.sh" -exec chmod +x {} \; 2>/dev/null || true
    
    log_success "Permissions set"
}

# 기본 설정 파일 생성
create_basic_config() {
    log_info "Creating basic configuration..."
    
    # 기본 딕셔너리 파일 생성
    if [ ! -f "configs/mm_load_dict.txt" ]; then
        mkdir -p configs
        cat > configs/mm_load_dict.txt << 'EOF'
# MM Load 관련 키워드들
"CFE_ES_Global"
"MM_AppData"
"/cf/apps/"
"/ram/"
"\x01\x00\x00\x00"
"\x02\x00\x00\x00"
EOF
    fi
    
    log_success "Basic configuration created"
}

# 메인 함수
main() {
    log_info "Starting CFS MM Fuzzer setup..."
    
    create_directories
    set_permissions
    create_basic_config
    
    log_success "Setup completed successfully!"
    log_info "Next steps:"
    log_info "  1. Build: ./scripts/build_all.sh"
    log_info "  2. Test: ./bin/release/mm_load_test"
    log_info "  3. Fuzz: ./scripts/run_fuzzing.sh --target mm_load"
}

main "$@"