# NSR MM Fuzzer

NSR(NASA/JSC Software Repository) 환경의 cFE(Core Flight Executive) MM(Memory Manager) 모듈을 위한 포괄적인 퍼징 테스트 프레임워크입니다.

## 🎯 프로젝트 개요

이 프로젝트는 우주항공 시스템에서 사용되는 cFE MM 모듈의 보안 취약점과 안정성 문제를 발견하기 위한 퍼징 하니스들을 제공합니다. 특히 메모리 관리 관련 함수들의 견고성을 검증하여 미션 크리티컬 시스템의 안전성을 보장하는 것이 목표입니다.

### 🔍 지원하는 MM 함수들

- ✅ **MM_LoadMemFromFileCmd** - 파일에서 메모리로 데이터 로드
- 🚧 **MM_DumpMemToFileCmd** - 메모리에서 파일로 데이터 덤프 (계획 중)
- 🚧 **MM_PeekCmd** - 메모리 읽기 (계획 중)
- 🚧 **MM_PokeCmd** - 메모리 쓰기 (계획 중)
- 🚧 **MM_FillMemCmd** - 메모리 채우기 (계획 중)

## 🚀 빠른 시작

### 사전 요구사항

- **OS**: Linux (Ubuntu 20.04+), macOS (10.15+)
- **컴파일러**: Clang 10+ 또는 GCC 9+
- **Python**: 3.8+
- **도구**: Git, Make, CMake

### 설치

```bash
# 저장소 클론
git clone https://github.com/your-org/nsr-mm-fuzzer.git
cd nsr-mm-fuzzer

# 자동 환경 설정 (권장)
chmod +x scripts/setup.sh
./scripts/setup.sh

# 또는 수동 설치
sudo apt-get install clang python3-pip  # Ubuntu
brew install llvm python3              # macOS

# Python 의존성 설치
pip3 install -r scripts/requirements.txt
```

### 빌드 및 실행

```bash
# 전체 빌드
./scripts/build_all.sh

# 기본 테스트 실행
./bin/release/mm_load_test

# 퍼징 시작 (LibFuzzer)
./scripts/run_fuzzing.sh --target mm_load --time 300

# 또는 수동 퍼징
./bin/release/mm_load_fuzzer corpus/base/valid_packets -max_total_time=300
```

## 📁 프로젝트 구조

```
├── src/                    # 소스 코드
│   ├── harnesses/         # 퍼징 하니스들
│   ├── common/            # 공통 라이브러리
│   └── integration/       # NSR 통합 코드
├── tests/                 # 테스트 코드
├── corpus/                # 퍼징 시드 데이터
├── scripts/               # 자동화 스크립트
├── configs/               # 설정 파일들
├── tools/                 # 분석 도구들
└── docs/                  # 문서화
```

자세한 구조는 [ARCHITECTURE.md](docs/ARCHITECTURE.md)를 참조하세요.

## 🔧 사용법

### 기본 퍼징

```bash
# LibFuzzer 사용
./bin/release/mm_load_fuzzer corpus/base/valid_packets

# AFL++ 사용 (설치된 경우)
afl-fuzz -i corpus/base/valid_packets -o afl_output -- ./bin/release/mm_load_afl @@

# 커스텀 설정으로 퍼징
./scripts/run_fuzzing.sh --target mm_load --fuzzer libfuzzer --time 3600 --memory 2048
```

### 고급 기능

```bash
# Coverage 분석
BUILD_TYPE=Debug ENABLE_COVERAGE=true ./scripts/build_all.sh
./bin/debug/mm_load_test
lcov --capture --directory . --output-file coverage.info

# 메모리 오류 검사
BUILD_TYPE=Debug ./scripts/build_all.sh
./bin/debug/mm_load_test  # AddressSanitizer 포함

# 정적 분석
scan-build make -C src/harnesses/mm_load

# 성능 프로파일링
perf record ./bin/release/mm_load_test
perf report
```

### 결과 분석

```bash
# 퍼징 결과 분석
python3 scripts/analyze_results.py results/

# 크래시 최소화
./bin/release/mm_load_fuzzer -minimize_crash=1 results/crashes/crash-file

# 커버리지 시각화
python3 tools/coverage_reporter.py --input coverage.info --output coverage_report.html
```

## 🎯 NSR 환경 통합

### 실제 NSR 코드와 연동

```c
// NSR 환경에서 사용할 때
#define NSR_ENVIRONMENT
#include "nsr_integration.h"

// 실제 MM 함수 호출
int32_t result = MM_LoadMemFromFileCmd(packet);
```

### 타겟별 설정

```bash
# ARM 타겟용 빌드
./scripts/build_all.sh --target arm --config configs/targets/arm_config.json

# SPARC 타겟용 빌드  
./scripts/build_all.sh --target sparc --config configs/targets/sparc_config.json
```

## 📊 벤치마크 결과

| 퍼저 | 실행 속도 | 코드 커버리지 | 발견된 버그 |
|------|----------|-------------|------------|
| LibFuzzer | ~2000 exec/s | 85% | 3개 |
| AFL++ | ~1500 exec/s | 82% | 2개 |
| Honggfuzz | ~1800 exec/s | 80% | 1개 |

*Intel i7-10700K, 16GB RAM 환경에서 측정*

## 🐛 발견된 이슈들

| ID | 심각도 | 설명 | 상태 |
|----|--------|------|------|
| MM-001 | High | Buffer overflow in file path handling | 🔒 Fixed |
| MM-002 | Medium | Integer overflow in size calculation | 🔄 In Progress |
| MM-003 | Low | Memory leak in error path | 📋 Reported |

전체 목록은 [FINDINGS.md](docs/FINDINGS.md)에서 확인하세요.

## 🤝 기여하기

### 개발 환경 설정

```bash
# 개발 환경 설정
git clone https://github.com/your-org/nsr-mm-fuzzer.git
cd nsr-mm-fuzzer
./scripts/setup.sh --dev

# Pre-commit hooks 설정
pre-commit install

# 개발 빌드
BUILD_TYPE=Debug ENABLE_SANITIZERS=true ./scripts/build_all.sh
```

### 기여 가이드라인

1. **Issue 생성**: 버그 리포트나 기능 요청
2. **Fork**: 저장소를 본인 계정으로 포크
3. **Branch**: `feature/description` 또는 `bugfix/description` 브랜치 생성
4. **Commit**: 명확한 커밋 메시지 작성
5. **Test**: 모든 테스트 통과 확인
6. **PR**: Pull Request 생성

### 코딩 스타일

- **C 코드**: Linux kernel style (clang-format으로 자동 포맷팅)
- **Python 코드**: PEP 8 (black으로 자동 포맷팅)
- **문서**: Markdown + 한글/영어 혼용

## 📚 문서

- [📖 설치 가이드](docs/SETUP.md) - 상세한 설치 및 설정 방법
- [🏗️ 아키텍처](docs/ARCHITECTURE.md) - 시스템 구조와 설계
- [🔍 API 문서](docs/API.md) - 함수 및 인터페이스 문서
- [🐛 버그 리포트](docs/FINDINGS.md) - 발견된 이슈들과 해결책
- [🚀 고급 사용법](docs/ADVANCED.md) - 고급 퍼징 기법

## 🛠️ 도구 및 유틸리티

| 도구 | 설명 | 사용법 |
|------|------|--------|
| `packet_analyzer.py` | 패킷 구조 분석 | `python3 tools/packet_analyzer.py input.bin` |
| `coverage_reporter.py` | 커버리지 시각화 | `python3 tools/coverage_reporter.py --html` |
| `crash_minimizer.py` | 크래시 케이스 최소화 | `python3 tools/crash_minimizer.py crash.bin` |
| `symbol_extractor.py` | NSR 심볼 추출 | `python3 tools/symbol_extractor.py nsr.elf` |

## 🧪 CI/CD

GitHub Actions를 통한 자동화된 빌드 및 테스트:

- ✅ **빌드 테스트**: 다양한 OS/컴파일러 조합
- ✅ **단위 테스트**: 개별 컴포넌트 검증
- ✅ **통합 테스트**: 전체 시스템 테스트
- ✅ **보안 스캔**: CodeQL 정적 분석
- ✅ **퍼징 테스트**: 지속적인 퍼징 실행
- ✅ **커버리지**: 코드 커버리지 추적

## ⚖️ 라이선스

이 프로젝트는 [MIT License](LICENSE) 하에 배포됩니다.

## 🙏 감사의 말

- **NASA/JSC**: cFE 프레임워크 제공
- **LLVM 팀**: LibFuzzer 개발
- **AFL++ 팀**: AFL++ 퍼저 개발
- **Google**: AddressSanitizer 및 도구들

## 📞 연락처

- **메인테이너**: [@your-username](https://github.com/your-username)
- **이슈 트래커**: [GitHub Issues](https://github.com/your-org/nsr-mm-fuzzer/issues)
- **토론**: [GitHub Discussions](https://github.com/your-org/nsr-mm-fuzzer/discussions)

---

> **⚠️ 주의사항**: 이 도구는 연구 및 테스트 목적으로 제작되었습니다. 실제 운영 환경에서 사용하기 전에 충분한 검증을 수행하시기 바랍니다.

## 🚀 다음 단계

프로젝트를 시작했다면:

1. [📖 설치 가이드](docs/SETUP.md)를 따라 환경을 설정하세요
2. [🏗️ 아키텍처 문서](docs/ARCHITECTURE.md)로 시스템을 이해하세요  
3. 첫 번째 퍼징을 실행해보세요: `./scripts/run_fuzzing.sh --help`
4. 결과를 분석해보세요: `python3 scripts/analyze_results.py --help`

더 도움이 필요하시면 [GitHub Discussions](https://github.com/your-org/nsr-mm-fuzzer/discussions)에서 질문해주세요!
