/*
 * CFS 설정 헤더 파일
 * Core Flight System 환경별 설정과 확장 기능
 */

#ifndef CFS_CONFIG_H
#define CFS_CONFIG_H

#include "cfe_mock.h"

// CFS 버전 정보
#define CFS_FUZZER_VERSION_MAJOR    1
#define CFS_FUZZER_VERSION_MINOR    0
#define CFS_FUZZER_VERSION_PATCH    0

// MM 모듈 관련 상수
#define MM_CMD_MID                    0x1888
#define MM_HK_TLM_MID                0x0887
#define MM_SEND_HK_MID               0x1889

// MM 명령 코드들
#define MM_NOOP_CC                   0
#define MM_RESET_CC                  1
#define MM_PEEK_CC                   2
#define MM_POKE_CC                   3
#define MM_LOAD_MEM_WID_CC          4
#define MM_LOAD_MEM_FROM_FILE_CC    5
#define MM_DUMP_MEM_TO_FILE_CC      6
#define MM_DUMP_IN_EVENT_CC         7
#define MM_FILL_MEM_CC              8
#define MM_LOOKUP_SYM_CC            9
#define MM_SYMTBL_TO_FILE_CC        10
#define MM_ENABLE_EEPROM_WRITE_CC   11
#define MM_DISABLE_EEPROM_WRITE_CC  12

// 메모리 타입 정의
#define MM_RAM                      1
#define MM_EEPROM                   2
#define MM_MEM8                     8
#define MM_MEM16                    16
#define MM_MEM32                    32

// 파일 이름 최대 길이
#ifndef OS_MAX_PATH_LEN
#define OS_MAX_PATH_LEN             64
#endif

#define MM_MAX_UNINTERRUPTIBLE_DATA 200
#define MM_MAX_LOAD_FILE_DATA_RAM   (1024 * 1024)  // 1MB
#define MM_MAX_LOAD_FILE_DATA_EEPROM (128 * 1024)  // 128KB
#define MM_MAX_DUMP_FILE_DATA_RAM   (1024 * 1024)  // 1MB
#define MM_MAX_DUMP_FILE_DATA_EEPROM (128 * 1024)  // 128KB
#define MM_MAX_FILL_DATA_RAM        (1024 * 1024)  // 1MB
#define MM_MAX_FILL_DATA_EEPROM     (128 * 1024)   // 128KB

// CFS 플랫폼별 설정
#ifdef CFS_TARGET_X86
    #define CFS_DEFAULT_RAM_ADDR      0x20000000
    #define CFS_DEFAULT_EEPROM_ADDR   0x08080000
    #define CFS_MAX_FILE_SIZE         (2 * 1024 * 1024)  // 2MB
    #define CFS_MEMORY_ALIGNMENT      4
#elif defined(CFS_TARGET_ARM)
    #define CFS_DEFAULT_RAM_ADDR      0x60000000  
    #define CFS_DEFAULT_EEPROM_ADDR   0x08100000
    #define CFS_MAX_FILE_SIZE         (1 * 1024 * 1024)  // 1MB
    #define CFS_MEMORY_ALIGNMENT      4
#elif defined(CFS_TARGET_SPARC)
    #define CFS_DEFAULT_RAM_ADDR      0x40000000
    #define CFS_DEFAULT_EEPROM_ADDR   0x30000000 
    #define CFS_MAX_FILE_SIZE         (512 * 1024)       // 512KB
    #define CFS_MEMORY_ALIGNMENT      8
#elif defined(CFS_TARGET_VXWORKS)
    #define CFS_DEFAULT_RAM_ADDR      0x20000000
    #define CFS_DEFAULT_EEPROM_ADDR   0x08000000
    #define CFS_MAX_FILE_SIZE         (1 * 1024 * 1024)  // 1MB
    #define CFS_MEMORY_ALIGNMENT      4
#else
    // 기본값 (시뮬레이션 환경)
    #define CFS_DEFAULT_RAM_ADDR      0x20000000
    #define CFS_DEFAULT_EEPROM_ADDR   0x08080000
    #define CFS_MAX_FILE_SIZE         (1 * 1024 * 1024)  // 1MB
    #define CFS_MEMORY_ALIGNMENT      4
#endif

// CFS 심볼들 (플랫폼별로 다를 수 있음)
static const char* cfs_valid_symbols[] = {
    // Core Flight Executive 기본 심볼들
    "CFE_ES_Global",
    "CFE_TBL_Global",
    "CFE_EVS_Global", 
    "CFE_SB_Global",
    "CFE_TIME_Global",
    
    // MM 모듈 관련
    "MM_AppData",
    "MM_ResetArea",
    "MM_CmdCounter",
    "MM_ErrCounter",
    
    // 다른 CFS 앱들
    "CS_AppData",      // Checksum 앱
    "DS_AppData",      // Data Storage 앱  
    "FM_AppData",      // File Manager 앱
    "HK_AppData",      // Housekeeping 앱
    "LC_AppData",      // Limit Checker 앱
    "MD_AppData",      // Memory Dwell 앱
    "SC_AppData",      // Stored Command 앱
    "SCH_AppData",     // Scheduler 앱
    
    // OSAL/PSP 심볼들
    "OS_VolumeTable",
    "PSP_MemoryTable",
    "CFE_PSP_MemoryTable",
    
    // 시스템 심볼들
    "BSP_RAM_START",
    "BSP_RAM_END", 
    "BSP_EEPROM_START",
    "BSP_EEPROM_END"
};

#define CFS_NUM_VALID_SYMBOLS (sizeof(cfs_valid_symbols) / sizeof(cfs_valid_symbols[0]))

// CFS 파일 시스템 경로들
static const char* cfs_valid_paths[] = {
    "/cf/apps/",           // 애플리케이션 파일들
    "/cf/download/",       // 다운로드된 파일들
    "/cf/upload/",         // 업로드할 파일들
    "/cf/log/",           // 로그 파일들
    "/cf/tmp/",           // 임시 파일들
    "/ram/",              // RAM 디스크
    "/rom/",              // ROM 파일 시스템
    "/vol/",              // 볼륨 마운트 포인트
    "/data/tables/",      // 테이블 파일들
    "/data/scripts/",     // 스크립트 파일들
    "/data/config/",      // 설정 파일들
    "/mission/apps/",     // 미션 특화 앱들
    "/mission/data/",     // 미션 데이터
    "/platform/boot/",    // 부트 파일들
    "/platform/config/"   // 플랫폼 설정
};

#define CFS_NUM_VALID_PATHS (sizeof(cfs_valid_paths) / sizeof(cfs_valid_paths[0]))

// CFS 메모리 맵 (플랫폼별로 다름)
typedef struct {
    uint64_t start_addr;
    uint64_t end_addr;
    uint8_t  mem_type;    // 1=RAM, 2=EEPROM, 3=FLASH, 4=IO
    char     name[32];
    boolean  writable;
    boolean  executable;
} CFS_MemoryRegion_t;

static const CFS_MemoryRegion_t cfs_memory_regions[] = {
    // RAM 영역들
    {0x20000000, 0x2007FFFF, MM_RAM, "SRAM_MAIN", TRUE, TRUE},
    {0x20080000, 0x200FFFFF, MM_RAM, "SRAM_BACKUP", TRUE, FALSE}, 
    {0x60000000, 0x67FFFFFF, MM_RAM, "SDRAM_EXTERNAL", TRUE, TRUE},
    
    // EEPROM 영역들  
    {0x08080000, 0x080FFFFF, MM_EEPROM, "EEPROM_CONFIG", TRUE, FALSE},
    {0x08100000, 0x081FFFFF, MM_EEPROM, "EEPROM_DATA", TRUE, FALSE},
    
    // ROM/FLASH 영역들 (읽기 전용)
    {0x08000000, 0x0807FFFF, 3, "FLASH_BOOT", FALSE, TRUE},
    {0x08200000, 0x087FFFFF, 3, "FLASH_APP", FALSE, TRUE},
};

#define CFS_NUM_MEMORY_REGIONS (sizeof(cfs_memory_regions) / sizeof(cfs_memory_regions[0]))

// CFS 유틸리티 함수들
static inline boolean CFS_IsValidMemoryAddress(uint64_t addr, uint8_t mem_type, uint32_t size) {
    for (int i = 0; i < CFS_NUM_MEMORY_REGIONS; i++) {
        const CFS_MemoryRegion_t *region = &cfs_memory_regions[i];
        if (region->mem_type == mem_type &&
            addr >= region->start_addr &&
            (addr + size - 1) <= region->end_addr) {
            return TRUE;
        }
    }
    return FALSE;
}

static inline const char* CFS_GetRandomSymbol(uint8_t seed) {
    return cfs_valid_symbols[seed % CFS_NUM_VALID_SYMBOLS];
}

static inline const char* CFS_GetRandomPath(uint8_t seed) {
    return cfs_valid_paths[seed % CFS_NUM_VALID_PATHS];
}

static inline uint32_t CFS_GetMaxDataSize(uint8_t mem_type) {
    switch (mem_type) {
        case MM_RAM:
            return MM_MAX_LOAD_FILE_DATA_RAM;
        case MM_EEPROM:
            return MM_MAX_LOAD_FILE_DATA_EEPROM;
        default:
            return 1024; // 기본값
    }
}

// 퍼징 모드 설정
#ifdef CFS_FUZZING_MODE
    // 퍼징 시에는 실제 파일/메모리 접근 방지
    #define CFS_ENABLE_FILE_ACCESS    FALSE
    #define CFS_ENABLE_MEMORY_ACCESS  FALSE
    #define CFS_SIMULATE_ONLY         TRUE
#else
    // 정상 운영 시에는 실제 동작
    #define CFS_ENABLE_FILE_ACCESS    TRUE
    #define CFS_ENABLE_MEMORY_ACCESS  TRUE  
    #define CFS_SIMULATE_ONLY         FALSE
#endif

// CFS 에러 코드 정의
#define CFS_MM_SUCCESS                 CFE_SUCCESS
#define CFS_MM_ERROR_INVALID_MEMTYPE  -1
#define CFS_MM_ERROR_INVALID_SIZE     -2
#define CFS_MM_ERROR_INVALID_ADDR     -3
#define CFS_MM_ERROR_INVALID_FILE     -4
#define CFS_MM_ERROR_FILE_ACCESS      -5
#define CFS_MM_ERROR_MEMORY_ACCESS    -6
#define CFS_MM_ERROR_CRC_MISMATCH     -7
#define CFS_MM_ERROR_RESOURCE_LIMIT   -8
#define CFS_MM_ERROR_ALIGNMENT        -9
#define CFS_MM_ERROR_PERMISSION       -10

// 미션별 확장 포인트
#ifdef CFS_MISSION_CUSTOM
    #include "mission_config.h"
#endif

#endif // CFS_CONFIG_H