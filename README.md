# CFS MM Fuzzer

[![CI Status](https://github.com/your-org/cfs-mm-fuzzer/workflows/CI/badge.svg)](https://github.com/your-org/cfs-mm-fuzzer/actions)
[![Coverage Status](https://codecov.io/gh/your-org/cfs-mm-fuzzer/branch/main/graph/badge.svg)](https://codecov.io/gh/your-org/cfs-mm-fuzzer)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Core Flight System (CFS) 환경의 cFE(Core Flight Executive) MM(Memory Manager) 모듈을 위한 포괄적인 퍼징 테스트 프레임워크입니다.

## 🎯 프로젝트 개요

이 프로젝트는 우주항공 시스템에서 사용되는 cFE MM 모듈의 보안 취약점과 안정성 문제를 발견하기 위한 퍼징 하니스들을 제공합니다. NASA의 Core Flight System (CFS) 환경에서 미션 크리티컬 시스템의 안전성을 보장하는 것이 목표입니다.

### 🔍 지원하는 MM 함수들

- ✅ **MM_LoadMemFromFileCmd** - 파일에서 메모리로 데이터 로드
- 🚧 **MM_DumpMemToFileCmd** - 메모리에서 파일로 데이터 덤프 (계획 중)
- 🚧 **MM_PeekCmd** - 메모리 읽기 (계획 중)
- 🚧 **MM_PokeCmd** - 메모리 쓰기 (계획 중)
- 🚧 **MM_FillMemCmd** - 메모리 채우기 (계획 중)

### 🌟 CFS 확장성

- **모듈화 설계**: 다른 CFS 앱들로 쉽게 확장 가능
- **플랫폼 독립**: x86, ARM, SPARC 등 다양한 타겟 지원
- **cFE 호환**: 표준 cFE 인터페이스 준수
- **미션 적응**: 특정 미션 요구사항에 맞게 커스터마이징 가능

## 🚀 빠른 시작

### 사전 요구사항

- **OS**: Linux (Ubuntu 20.04+), macOS (10.15+), VxWorks 7.0+
- **컴파일러**: Clang 10+ 또는 GCC 9+
- **Python**: 3.8+
- **CFS**: cFE 6.7+ (선택사항, 통합 테스트용)

### 설치

```bash
# 저장소 클론
git clone https://github.com/your-org/cfs-mm-fuzzer.git
cd cfs-mm-fuzzer

# 자동 환경 설정 (권장)
chmod +x scripts/setup.sh
./scripts/setup.sh

# CFS 환경과 통합 (선택사항)
export CFS_ROOT=/path/to/your/cfs
./scripts/setup.sh --with-cfs
```

### 빌드 및 실행

```bash
# 전체 빌드
./scripts/build_all.sh

# 기본 테스트 실행
./bin/release/mm_load_test

# 퍼징 시작 (LibFuzzer)
./scripts/run_fuzzing.sh --target mm_load --time 300

# CFS 통합 모드
./scripts/run_fuzzing.sh --target mm_load --cfs-mode --time 300
```

더 자세한 정보는 [docs/](docs/) 디렉토리를 참조하세요.
