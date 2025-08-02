/*
 * CFS MM Fuzzer 공통 유틸리티 함수들
 */

#include "utils.h"
#include <time.h>

// 전역 변수들
static boolean g_debug_enabled = FALSE;
static FILE *g_log_file = NULL;

// 기본 MM 설정
const MM_Config_t MM_DefaultConfig = {
    .max_ram_load_size = MM_MAX_LOAD_FILE_DATA_RAM,
    .max_eeprom_load_size = MM_MAX_LOAD_FILE_DATA_EEPROM,
    .max_ram_dump_size = MM_MAX_DUMP_FILE_DATA_RAM,
    .max_eeprom_dump_size = MM_MAX_DUMP_FILE_DATA_EEPROM,
    .max_fill_size = MM_MAX_FILL_DATA_RAM,
    .enable_crc_check = TRUE,
    .enable_eeprom_write = FALSE,
    .crc_function = MM_CalculateCRC32,
    .file_load_callback = NULL,
    .file_dump_callback = NULL
};

/*
 * 심볼 주소 해석 함수
 */
int32_t MM_ResolveSymAddr(MM_SymAddr_t *SymAddr, uint64_t *ResolvedAddr) {
    if (!SymAddr || !ResolvedAddr) {
        return OS_INVALID_POINTER;
    }
    
    // 심볼 이름이 있는 경우
    if (strlen(SymAddr->SymName) > 0) {
        uint64_t symbol_addr = 0;
        int32_t status = OS_SymbolLookup(&symbol_addr, SymAddr->SymName);
        if (status != OS_SUCCESS) {
            CFS_DEBUG_PRINT("Symbol lookup failed for: %s", SymAddr->SymName);
            return status;
        }
        *ResolvedAddr = symbol_addr + SymAddr->Offset;
    } else {
        // 직접 주소 사용
        *ResolvedAddr = SymAddr->Offset;
    }
    
    CFS_DEBUG_PRINT("Resolved address: 0x%08lX", *ResolvedAddr);
    return CFE_SUCCESS;
}

/*
 * 메모리 타입 검증
 */
int32_t MM_VerifyMemType(uint8_t MemType) {
    switch (MemType) {
        case MM_RAM:
        case MM_EEPROM:
        case MM_MEM8:
        case MM_MEM16:
        case MM_MEM32:
            return CFE_SUCCESS;
        default:
            CFS_DEBUG_PRINT("Invalid memory type: %d", MemType);
            return CFS_MM_ERROR_INVALID_MEMTYPE;
    }
}

/*
 * 데이터 크기 검증
 */
int32_t MM_VerifyDataSize(uint8_t DataSize) {
    switch (DataSize) {
        case MM_MEM8:
        case MM_MEM16:
        case MM_MEM32:
            return CFE_SUCCESS;
        default:
            CFS_DEBUG_PRINT("Invalid data size: %d", DataSize);
            return CFS_MM_ERROR_INVALID_SIZE;
    }
}

/*
 * 파일 이름 검증
 */
int32_t MM_VerifyFileName(const char *FileName) {
    if (!FileName) {
        return OS_INVALID_POINTER;
    }
    
    size_t len = strlen(FileName);
    if (len == 0 || len >= OS_MAX_PATH_LEN) {
        CFS_DEBUG_PRINT("Invalid filename length: %zu", len);
        return CFS_MM_ERROR_INVALID_FILE;
    }
    
    // 경로 순회 공격 방지
    if (strstr(FileName, "../") || strstr(FileName, "..\\")) {
        CFS_DEBUG_PRINT("Path traversal detected in filename: %s", FileName);
        return CFS_MM_ERROR_INVALID_FILE;
    }
    
    // 허용된 경로인지 확인
    boolean valid_path = FALSE;
    for (int i = 0; i < CFS_NUM_VALID_PATHS; i++) {
        if (strncmp(FileName, cfs_valid_paths[i], strlen(cfs_valid_paths[i])) == 0) {
            valid_path = TRUE;
            break;
        }
    }
    
    if (!valid_path) {
        CFS_DEBUG_PRINT("Filename not in allowed paths: %s", FileName);
        return CFS_MM_ERROR_INVALID_FILE;
    }
    
    return CFE_SUCCESS;
}

/*
 * 메모리 정렬 검증
 */
boolean MM_IsValidAlignment(uint64_t addr, uint8_t data_size) {
    uint32_t alignment = 1;
    
    switch (data_size) {
        case MM_MEM8:
            alignment = 1;
            break;
        case MM_MEM16:
            alignment = 2;
            break;
        case MM_MEM32:
            alignment = 4;
            break;
        default:
            return FALSE;
    }
    
    return (addr % alignment) == 0;
}

/*
 * CRC32 계산 함수
 */
uint32_t MM_CalculateCRC32(const void *data, size_t length, uint32_t initial_crc) {
    if (!data || length == 0) {
        return initial_crc;
    }
    
    // 간단한 CRC32 구현 (실제로는 더 정교한 알고리즘 사용)
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc = initial_crc;
    
    static const uint32_t crc_table[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
        0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
        0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91
        // ... 나머지 테이블 생략 (실제 구현에서는 전체 256 엔트리 필요)
    };
    
    for (size_t i = 0; i < length; i++) {
        crc = crc_table[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
    }
    
    return crc;
}

/*
 * 일반적인 CRC 계산 래퍼
 */
int32_t MM_CalculateCRC(const void *data, size_t length, uint32_t *crc) {
    if (!data || !crc) {
        return OS_INVALID_POINTER;
    }
    
    *crc = MM_CalculateCRC32(data, length, 0xFFFFFFFF);
    return CFE_SUCCESS;
}

/*
 * 랜덤 데이터 생성 유틸리티
 */
void CFS_GenerateRandomData(uint8_t *buffer, size_t size, uint32_t seed) {
    if (!buffer || size == 0) {
        return;
    }
    
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (uint8_t)(rand() & 0xFF);
    }
}

/*
 * 패킷 헤더 초기화
 */
void CFS_InitPacketHeader(CFE_MSG_Message_t *MsgPtr, CFE_SB_MsgId_t MsgId, 
                         uint16_t Length, uint8_t FcnCode) {
    if (!MsgPtr) {
        return;
    }
    
    memset(MsgPtr, 0, sizeof(CFE_MSG_CommandHeader_t));
    
    CFE_MSG_SetMsgId(MsgPtr, MsgId);
    CFE_MSG_SetSize(MsgPtr, Length);
    CFE_MSG_SetSequenceCount(MsgPtr, 0);
    CFE_MSG_SetSegmentationFlag(MsgPtr, 0x3); // Complete packet
    CFE_MSG_SetFcnCode(MsgPtr, FcnCode);
    
    CFS_DEBUG_PRINT("Packet header initialized: MsgId=0x%04X, Length=%d, FcnCode=%d", 
                     MsgId.Value, Length, FcnCode);
}

/*
 * 16진수 덤프 유틸리티
 */
void CFS_HexDump(const void *data, size_t length, const char *prefix) {
    if (!data || length == 0) {
        return;
    }
    
    const uint8_t *bytes = (const uint8_t *)data;
    
    for (size_t i = 0; i < length; i += 16) {
        if (prefix) {
            printf("%s%04zX: ", prefix, i);
        } else {
            printf("%04zX: ", i);
        }
        
        // 16진수 출력
        for (size_t j = 0; j < 16; j++) {
            if (i + j < length) {
                printf("%02X ", bytes[i + j]);
            } else {
                printf("   ");
            }
        }
        
        printf(" ");
        
        // ASCII 출력
        for (size_t j = 0; j < 16 && i + j < length; j++) {
            uint8_t c = bytes[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("\n");
    }
}

/*
 * 메모리 영역 유효성 검사
 */
int32_t CFS_ValidateMemoryRegion(uint64_t addr, uint32_t size, uint8_t mem_type, 
                                boolean write_access) {
    // 주소가 유효한 범위에 있는지 확인
    if (!CFS_IsValidMemoryAddress(addr, mem_type, size)) {
        CFS_DEBUG_PRINT("Invalid memory region: addr=0x%08lX, size=%u, type=%d", 
                         addr, size, mem_type);
        return CFS_MM_ERROR_INVALID_ADDR;
    }
    
    // 해당 영역의 쓰기 권한 확인
    if (write_access) {
        for (int i = 0; i < CFS_NUM_MEMORY_REGIONS; i++) {
            const CFS_MemoryRegion_t *region = &cfs_memory_regions[i];
            if (region->mem_type == mem_type &&
                addr >= region->start_addr &&
                (addr + size - 1) <= region->end_addr) {
                if (!region->writable) {
                    CFS_DEBUG_PRINT("Write access denied to read-only region: %s", 
                                     region->name);
                    return CFS_MM_ERROR_PERMISSION;
                }
                break;
            }
        }
    }
    
    return CFE_SUCCESS;
}

/*
 * 디버그 로깅 함수들
 */
void CFS_EnableDebug(boolean enable) {
    g_debug_enabled = enable;
}

void CFS_SetLogFile(const char *filename) {
    if (g_log_file && g_log_file != stdout && g_log_file != stderr) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
    
    if (filename) {
        g_log_file = fopen(filename, "a");
        if (!g_log_file) {
            g_log_file = stdout;
        }
    } else {
        g_log_file = stdout;
    }
}

void CFS_LogMessage(const char *level, const char *format, ...) {
    if (!g_debug_enabled) {
        return;
    }
    
    if (!g_log_file) {
        g_log_file = stdout;
    }
    
    // 타임스탬프 추가
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    
    fprintf(g_log_file, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] ",
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, level);
    
    va_list args;
    va_start(args, format);
    vfprintf(g_log_file, format, args);
    va_end(args);
    
    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

/*
 * 퍼징 통계 관련 함수들
 */
static CFS_FuzzStats_t g_fuzz_stats = {0};

void CFS_InitFuzzStats(void) {
    memset(&g_fuzz_stats, 0, sizeof(CFS_FuzzStats_t));
    g_fuzz_stats.start_time = time(NULL);
}

void CFS_UpdateFuzzStats(CFS_FuzzResult_t result) {
    g_fuzz_stats.total_executions++;
    
    switch (result) {
        case CFS_FUZZ_SUCCESS:
            g_fuzz_stats.successful_executions++;
            break;
        case CFS_FUZZ_ERROR:
            g_fuzz_stats.error_executions++;
            break;
        case CFS_FUZZ_CRASH:
            g_fuzz_stats.crash_executions++;
            break;
        case CFS_FUZZ_TIMEOUT:
            g_fuzz_stats.timeout_executions++;
            break;
        default:
            break;
    }
}

const CFS_FuzzStats_t* CFS_GetFuzzStats(void) {
    g_fuzz_stats.elapsed_time = time(NULL) - g_fuzz_stats.start_time;
    return &g_fuzz_stats;
}

void CFS_PrintFuzzStats(void) {
    const CFS_FuzzStats_t *stats = CFS_GetFuzzStats();
    
    printf("\n=== CFS Fuzzing Statistics ===\n");
    printf("Total Executions: %lu\n", stats->total_executions);
    printf("Successful: %lu (%.2f%%)\n", stats->successful_executions,
           stats->total_executions > 0 ? (100.0 * stats->successful_executions / stats->total_executions) : 0.0);
    printf("Errors: %lu (%.2f%%)\n", stats->error_executions,
           stats->total_executions > 0 ? (100.0 * stats->error_executions / stats->total_executions) : 0.0);
    printf("Crashes: %lu (%.2f%%)\n", stats->crash_executions,
           stats->total_executions > 0 ? (100.0 * stats->crash_executions / stats->total_executions) : 0.0);
    printf("Timeouts: %lu (%.2f%%)\n", stats->timeout_executions,
           stats->total_executions > 0 ? (100.0 * stats->timeout_executions / stats->total_executions) : 0.0);
    printf("Elapsed Time: %lu seconds\n", stats->elapsed_time);
    printf("Execution Rate: %.2f exec/sec\n",
           stats->elapsed_time > 0 ? ((double)stats->total_executions / stats->elapsed_time) : 0.0);
    printf("=============================\n\n");
}