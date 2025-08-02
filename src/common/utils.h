/*
 * CFS MM Fuzzer 공통 유틸리티 헤더
 */

#ifndef CFS_UTILS_H
#define CFS_UTILS_H

#include "mm_types.h"
#include <stdarg.h>
#include <time.h>

// 퍼징 결과 타입
typedef enum {
    CFS_FUZZ_SUCCESS = 0,
    CFS_FUZZ_ERROR,
    CFS_FUZZ_CRASH,
    CFS_FUZZ_TIMEOUT,
    CFS_FUZZ_INVALID_INPUT
} CFS_FuzzResult_t;

// 퍼징 통계 구조체
typedef struct {
    uint64_t total_executions;
    uint64_t successful_executions;
    uint64_t error_executions;
    uint64_t crash_executions;
    uint64_t timeout_executions;
    time_t   start_time;
    time_t   elapsed_time;
} CFS_FuzzStats_t;

// MM 함수 프로토타입들
int32_t MM_ResolveSymAddr(MM_SymAddr_t *SymAddr, uint64_t *ResolvedAddr);
int32_t MM_VerifyMemType(uint8_t MemType);
int32_t MM_VerifyDataSize(uint8_t DataSize);
int32_t MM_VerifyFileName(const char *FileName);
int32_t MM_CalculateCRC(const void *data, size_t length, uint32_t *crc);
boolean MM_IsValidAlignment(uint64_t addr, uint8_t data_size);

// CRC 계산 함수들
uint32_t MM_CalculateCRC32(const void *data, size_t length, uint32_t initial_crc);

// CFS 유틸리티 함수들
void CFS_GenerateRandomData(uint8_t *buffer, size_t size, uint32_t seed);
void CFS_InitPacketHeader(CFE_MSG_Message_t *MsgPtr, CFE_SB_MsgId_t MsgId, 
                         uint16_t Length, uint8_t FcnCode);
void CFS_HexDump(const void *data, size_t length, const char *prefix);
int32_t CFS_ValidateMemoryRegion(uint64_t addr, uint32_t size, uint8_t mem_type, 
                                boolean write_access);

// 디버그 및 로깅 함수들
void CFS_EnableDebug(boolean enable);
void CFS_SetLogFile(const char *filename);
void CFS_LogMessage(const char *level, const char *format, ...);

// 퍼징 통계 함수들
void CFS_InitFuzzStats(void);
void CFS_UpdateFuzzStats(CFS_FuzzResult_t result);
const CFS_FuzzStats_t* CFS_GetFuzzStats(void);
void CFS_PrintFuzzStats(void);

// 디버그 매크로들
#ifdef CFS_DEBUG_MODE
    #define CFS_DEBUG_PRINT(fmt, ...) CFS_LogMessage("DEBUG", fmt, ##__VA_ARGS__)
    #define CFS_INFO_PRINT(fmt, ...) CFS_LogMessage("INFO", fmt, ##__VA_ARGS__)
    #define CFS_WARN_PRINT(fmt, ...) CFS_LogMessage("WARN", fmt, ##__VA_ARGS__)
    #define CFS_ERROR_PRINT(fmt, ...) CFS_LogMessage("ERROR", fmt, ##__VA_ARGS__)
#else
    #define CFS_DEBUG_PRINT(fmt, ...)
    #define CFS_INFO_PRINT(fmt, ...)
    #define CFS_WARN_PRINT(fmt, ...)
    #define CFS_ERROR_PRINT(fmt, ...)
#endif

// 전역 변수 선언 (extern)
extern const MM_Config_t MM_DefaultConfig;

#endif // CFS_UTILS_H