/*
 * MM 모듈 타입 정의
 * Memory Manager 관련 구조체와 타입들
 */

#ifndef MM_TYPES_H
#define MM_TYPES_H

#include "cfe_mock.h"
#include "cfs_config.h"

// MM 앱 데이터 구조체 (간소화된 버전)
typedef struct {
    CFE_MSG_CommandHeader_t  CmdHeader;    // 명령 헤더
    uint16                   CmdCounter;   // 명령 카운터
    uint16                   ErrCounter;   // 에러 카운터
    uint8                    LastAction;   // 마지막 수행 작업
    uint8                    MemType;      // 메모리 타입
    uint32                   Address;      // 메모리 주소
    uint32                   DataValue;    // 데이터 값
    uint32                   BytesProcessed; // 처리된 바이트 수
    char                     FileName[OS_MAX_PATH_LEN]; // 파일 이름
} MM_AppData_t;

// MM 심볼 주소 구조체
typedef struct {
    char     SymName[OS_MAX_API_NAME];   // 심볼 이름
    uint64_t Offset;                     // 오프셋
} MM_SymAddr_t;

// MM Load Memory From File 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
    uint8                   MemType;       // 메모리 타입 (MM_RAM, MM_EEPROM 등)
    uint32                  NumOfBytes;    // 로드할 바이트 수
    uint32                  CRC;           // 파일 CRC (옵션)
    MM_SymAddr_t           DestSymAddress; // 목적지 심볼 주소
    char                    FileName[OS_MAX_PATH_LEN]; // 소스 파일 이름
} MM_LoadMemFromFileCmd_t;

// MM Dump Memory To File 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
    uint8                   MemType;       // 메모리 타입
    uint32                  NumOfBytes;    // 덤프할 바이트 수
    MM_SymAddr_t           SrcSymAddress;  // 소스 심볼 주소
    char                    FileName[OS_MAX_PATH_LEN]; // 목적지 파일 이름
} MM_DumpMemToFileCmd_t;

// MM Peek 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
    uint8                   DataSize;      // 데이터 크기 (8, 16, 32 비트)
    uint8                   MemType;       // 메모리 타입
    MM_SymAddr_t           SrcSymAddress;  // 소스 심볼 주소
} MM_PeekCmd_t;

// MM Poke 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
    uint8                   DataSize;      // 데이터 크기 (8, 16, 32 비트)
    uint8                   MemType;       // 메모리 타입
    uint32                  Data;          // 쓸 데이터
    MM_SymAddr_t           DestSymAddress; // 목적지 심볼 주소
} MM_PokeCmd_t;

// MM Fill Memory 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
    uint8                   MemType;       // 메모리 타입
    uint32                  NumOfBytes;    // 채울 바이트 수
    uint32                  FillPattern;   // 채울 패턴
    MM_SymAddr_t           DestSymAddress; // 목적지 심볼 주소
} MM_FillMemCmd_t;

// MM Lookup Symbol 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
    char                    SymName[OS_MAX_API_NAME]; // 심볼 이름
} MM_LookupSymCmd_t;

// MM No-Op 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
} MM_NoopCmd_t;

// MM Reset 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
} MM_ResetCmd_t;

// MM EEPROM Write Enable/Disable 명령 구조체
typedef struct {
    CFE_MSG_CommandHeader_t CmdHeader;     // 표준 cFE 명령 헤더
    uint32                  Bank;          // EEPROM 뱅크 번호
} MM_EepromWriteEnaCmd_t;

typedef MM_EepromWriteEnaCmd_t MM_EepromWriteDisCmd_t;

// MM 하우스키핑 텔레메트리 구조체
typedef struct {
    CFE_MSG_CommandHeader_t TlmHeader;     // 표준 cFE 텔레메트리 헤더
    uint8                   CmdCounter;    // 명령 카운터
    uint8                   ErrCounter;    // 에러 카운터
    uint8                   LastAction;    // 마지막 수행 작업
    uint8                   MemType;       // 메모리 타입
    uint32                  Address;       // 메모리 주소
    uint32                  DataValue;     // 데이터 값 (Peek 결과)
    uint32                  BytesProcessed; // 처리된 바이트 수
    char                    FileName[OS_MAX_PATH_LEN]; // 파일 이름
} MM_HkPacket_t;

// MM 이벤트 ID 정의
#define MM_STARTUP_INF_EID              1
#define MM_COMMAND_ERR_EID              2
#define MM_COMMANDNOP_INF_EID           3
#define MM_COMMANDRST_INF_EID           4
#define MM_INVALID_MSGID_ERR_EID        5
#define MM_CC1_ERR_EID                  6
#define MM_PEEK_BYTE_INF_EID            10
#define MM_PEEK_WORD_INF_EID            11
#define MM_PEEK_DWORD_INF_EID           12
#define MM_POKE_BYTE_INF_EID            20
#define MM_POKE_WORD_INF_EID            21
#define MM_POKE_DWORD_INF_EID           22
#define MM_LOAD_FROM_FILE_INF_EID       30
#define MM_LOAD_FROM_FILE_ERR_EID       31
#define MM_DUMP_TO_FILE_INF_EID         40
#define MM_DUMP_TO_FILE_ERR_EID         41
#define MM_FILL_INF_EID                 50
#define MM_FILL_ERR_EID                 51
#define MM_SYMNAME_ERR_EID              60
#define MM_SYMADDR_ERR_EID              61
#define MM_FILENAME_ERR_EID             70
#define MM_FILESIZE_ERR_EID             71
#define MM_FILE_ACCESS_ERR_EID          72

// MM 액션 타입 정의
#define MM_NOACTION                     0
#define MM_PEEK                         1
#define MM_POKE                         2
#define MM_LOAD_FROM_FILE               3
#define MM_DUMP_TO_FILE                 4
#define MM_FILL                         5
#define MM_LOOKUP_SYM                   6

// 유틸리티 매크로들
#define MM_CMD_HEADER_SIZE              sizeof(CFE_MSG_CommandHeader_t)
#define MM_TLM_HEADER_SIZE              sizeof(CFE_MSG_CommandHeader_t)

// 메모리 정렬 매크로
#define MM_ALIGN_TO_BOUNDARY(addr, boundary) \
    (((addr) + (boundary) - 1) & ~((boundary) - 1))

// 메모리 범위 검증 매크로
#define MM_VERIFY_MEM_RANGE(addr, size, mem_type) \
    CFS_IsValidMemoryAddress(addr, mem_type, size)

// 파일 이름 검증 매크로
#define MM_VERIFY_FILE_NAME(filename) \
    ((filename != NULL) && (strlen(filename) > 0) && (strlen(filename) < OS_MAX_PATH_LEN))

// CRC 계산 함수 타입
typedef uint32_t (*MM_CRC_Function_t)(const void *data, size_t length, uint32_t initial_crc);

// MM 콜백 함수 타입들
typedef int32_t (*MM_FileLoadCallback_t)(const char *filename, void *dest_addr, uint32_t size);
typedef int32_t (*MM_FileDumpCallback_t)(const char *filename, const void *src_addr, uint32_t size);

// MM 설정 구조체
typedef struct {
    uint32_t max_ram_load_size;
    uint32_t max_eeprom_load_size;
    uint32_t max_ram_dump_size;
    uint32_t max_eeprom_dump_size;
    uint32_t max_fill_size;
    boolean  enable_crc_check;
    boolean  enable_eeprom_write;
    MM_CRC_Function_t crc_function;
    MM_FileLoadCallback_t file_load_callback;
    MM_FileDumpCallback_t file_dump_callback;
} MM_Config_t;

// 기본 MM 설정
extern const MM_Config_t MM_DefaultConfig;

// MM 유틸리티 함수 프로토타입
int32_t MM_ResolveSymAddr(MM_SymAddr_t *SymAddr, uint64_t *ResolvedAddr);
int32_t MM_VerifyMemType(uint8_t MemType);
int32_t MM_VerifyDataSize(uint8_t DataSize);
int32_t MM_VerifyFileName(const char *FileName);
int32_t MM_CalculateCRC(const void *data, size_t length, uint32_t *crc);
boolean MM_IsValidAlignment(uint64_t addr, uint8_t data_size);

#endif // MM_TYPES_H