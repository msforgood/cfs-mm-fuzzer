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
chmod +x scripts/build_all.sh
./scripts/build_all.sh

# 기본 테스트 실행
./bin/release/mm_load_test

# 퍼징 시작 (LibFuzzer)
chmod +x scripts/run_fuzzing.sh
./scripts/run_fuzzing.sh --target mm_load --time 300

# CFS 통합 모드
./scripts/run_fuzzing.sh --target mm_load --cfs-mode --time 300
```

## 📁 프로젝트 구조

```
cfs-mm-fuzzer/
├── src/                    # 소스 코드
│   ├── harnesses/         # 퍼징 하니스들
│   │   ├── mm_load/       # MM Load 하니스 ✅
│   │   ├── mm_dump/       # MM Dump 하니스 🚧
│   │   ├── mm_peek/       # MM Peek 하니스 🚧
│   │   └── mm_fill/       # MM Fill 하니스 🚧
│   ├── common/            # 공통 라이브러리
│   │   ├── cfe_mock.h     # cFE Mock 정의
│   │   ├── cfs_config.h   # CFS 설정
│   │   ├── mm_types.h     # MM 타입 정의
│   │   └── utils.c/h      # 유틸리티 함수들
│   └── integration/       # CFS 통합 코드
├── tests/                 # 테스트 코드
├── corpus/               # 퍼징 시드 데이터
├── scripts/              # 자동화 스크립트
├── configs/              # 설정 파일들
├── tools/                # 분석 도구들
└── docs/                 # 문서화
```

## 🔧 사용법

### 기본 퍼징

```bash
# LibFuzzer 사용 (기본)
./scripts/run_fuzzing.sh --target mm_load --time 300

# AFL++ 사용
./scripts/run_fuzzing.sh --target mm_load --fuzzer afl --time 1800

# 병렬 퍼징
./scripts/run_fuzzing.sh --target mm_load --jobs 4 --time 3600

# 지속적 퍼징 (24시간)
./scripts/run_fuzzing.sh --target mm_load --continuous --time 86400
```

### CFS 통합 모드

```bash
# CFS 환경과 연동
export CFS_ROOT=/opt/cfs
./scripts/run_fuzzing.sh --target mm_load --cfs-mode

# 특정 미션 설정
./scripts/run_fuzzing.sh --target mm_load --mission-config configs/missions/your_mission.json
```

### 고급 기능

```bash
# Debug 빌드로 퍼징
BUILD_TYPE=Debug ./scripts/build_all.sh
./scripts/run_fuzzing.sh --build-type debug

# Coverage 분석
BUILD_TYPE=Debug ENABLE_COVERAGE=true ./scripts/build_all.sh
cd src/harnesses/mm_load && make coverage-report

# 메모리 검사
cd src/harnesses/mm_load && make memcheck

# 정적 분석
cd src/harnesses/mm_load && make static-analysis
```

## 📊 벤치마크 결과

| 퍼저 | 플랫폼 | 실행 속도 | 메모리 사용량 | 발견된 이슈 |
|------|--------|----------|-------------|------------|
| LibFuzzer | Linux x86 | ~2000 exec/s | 50MB | 3개 |
| AFL++ | Linux x86 | ~1500 exec/s | 80MB | 2개 |
| LibFuzzer | ARM | ~800 exec/s | 32MB | 1개 |

*Intel i7-10700K, 16GB RAM 환경에서 측정*

## 🐛 발견된 이슈들

| ID | 심각도 | 설명 | 상태 | 하니스 |
|----|--------|------|------|--------|
| MM-001 | High | Buffer overflow in filename handling | 🔒 Fixed | mm_load |
| MM-002 | Medium | Integer overflow in size calculation | 🔄 In Progress | mm_load |
| MM-003 | Low | Path traversal in file validation | 📋 Reported | mm_load |

## 🎯 CFS 특화 기능

### 1. cFE 인터페이스 호환성

```c
// 표준 cFE 메시지 인터페이스 지원
CFE_MSG_CommandHeader_t *CmdPtr = (CFE_MSG_CommandHeader_t *)PacketPtr;
CFE_MSG_GetMsgId(&MsgId, CmdPtr);
CFE_MSG_GetFcnCode(&FcnCode, CmdPtr);
```

### 2. 미션별 커스터마이징

```json
{
  "mission": {
    "name": "YOUR_MISSION",
    "memory_layout": {
      "ram_start": "0x20000000",
      "ram_size": "0x8000000"
    },
    "symbols": [
      "YOUR_APP_Global",
      "MISSION_SPECIFIC_Symbol"
    ]
  }
}
```

## 🤝 기여하기

### 개발 환경 설정

```bash
# 개발 환경 설정
./scripts/setup.sh --dev

# 개발 빌드
BUILD_TYPE=Debug ENABLE_SANITIZERS=true ./scripts/build_all.sh

# 테스트 실행
./scripts/run_fuzzing.sh --fuzzer standalone
```

## 📚 문서

- [📖 설치 가이드](docs/SETUP.md) - 상세한 설치 및 설정 방법
- [🏗️ CFS 통합](docs/CFS_INTEGRATION.md) - CFS 환경 통합 방법
- [🔍 API 문서](docs/API.md) - 함수 및 인터페이스 문서
- [🐛 버그 리포트](docs/FINDINGS.md) - 발견된 이슈들과 해결책

## 🛠️ 도구 및 유틸리티

| 도구 | 설명 | 사용법 |
|------|------|--------|
| `build_all.sh` | 전체 빌드 스크립트 | `./scripts/build_all.sh` |
| `run_fuzzing.sh` | 퍼징 실행 스크립트 | `./scripts/run_fuzzing.sh --help` |
| `setup.sh` | 환경 설정 스크립트 | `./scripts/setup.sh` |

## ⚖️ 라이선스

이 프로젝트는 [MIT License](LICENSE) 하에 배포됩니다.

## 🙏 감사의 말

- **NASA Goddard**: cFE/CFS 프레임워크 제공
- **NASA Johnson**: Core Flight System 개발
- **LLVM 팀**: LibFuzzer 개발
- **AFL++ 팀**: AFL++ 퍼저 개발

## 📞 연락처

- **메인테이너**: [@cfs-fuzzer-team](https://github.com/cfs-fuzzer-team)
- **이슈 트래커**: [GitHub Issues](https://github.com/your-org/cfs-mm-fuzzer/issues)
- **토론**: [GitHub Discussions](https://github.com/your-org/cfs-mm-fuzzer/discussions)

---

> **🛸 우주항공 알림**: 이 도구는 우주항공 시스템의 안전성 향상을 위해 제작되었습니다. 실제 우주 미션에서 사용하기 전에 충분한 검증과 인증 과정을 거치시기 바랍니다.

## 🚀 다음 단계

프로젝트를 시작했다면:

1. [📖 설치 가이드](docs/SETUP.md)를 따라 환경을 설정하세요
2. 첫 번째 빌드를 실행하세요: `./scripts/build_all.sh`
3. 기본 테스트를 실행하세요: `./bin/release/mm_load_test`
4. 첫 번째 퍼징을 시작하세요: `./scripts/run_fuzzing.sh --target mm_load --time 60`

더 도움이 필요하시면 [GitHub Issues](https://github.com/your-org/cfs-mm-fuzzer/issues)에서 질문해주세요!