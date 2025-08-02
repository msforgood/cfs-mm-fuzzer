/*
 * CFS CFE Mock 헤더 파일
 * Core Flight Executive의 기본 타입과 함수들을 Mock으로 정의
 */

#ifndef CFS_CFE_MOCK_H
#define CFS_CFE_MOCK_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// CFS 기본 타입 정의
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

// Boolean 타입
typedef uint8 boolean;
#define TRUE  1
#define FALSE 0

// CFE 상수 정의
#define CFE_SUCCESS                    0
#define CFE_SEVERITY_INFORMATION       1
#define CFE_SEVERITY_ERROR            4

// OS 관련 상수
#define OS_MAX_PATH_LEN               64
#define OS_MAX_API_NAME               20
#define OS_SUCCESS                    0
#define OS_ERROR                     (-1)
#define OS_INVALID_POINTER           (-2)
#define OS_ERROR_ADDRESS_MISALIGNED  (-3)
#define OS_ERROR_TIMEOUT             (-4)
#define OS_QUEUE_FULL                (-5)
#define OS_QUEUE_EMPTY               (-6)

// CFE Software Bus 관련
#define CFE_SB_MAX_PIPE_DEPTH        64
#define CFE_SB_POLL                  0
#define CFE_SB_PEND_FOREVER         -1

// CFE 메시지 관련 구조체
typedef struct {
    uint16_t StreamId;
    uint16_t Sequence;
    uint16_t Length;
    uint16_t Spare;
} CFE_MSG_CommandHeader_t;

typedef struct {
    CFE_MSG_CommandHeader_t Hdr;
} CFE_MSG_Message_t;

typedef CFE_MSG_Message_t* CFE_SB_MsgPtr_t;
typedef CFE_MSG_Message_t* CFE_SB_Buffer_t;

// CFE 메시지 ID 타입
typedef struct {
    uint32_t Value;
} CFE_SB_MsgId_t;

// CFE 파이프 ID
typedef uint32_t CFE_SB_PipeId_t;

// CFE 심볼 주소 구조체
typedef struct {
    char     SymName[OS_MAX_API_NAME];
    uint64_t Offset;
} CFS_SymAddr_t;

// CFE Event Services 관련
typedef uint32_t CFE_EVS_EventType_Enum_t;
typedef uint16_t CFE_EVS_EventID_t;

// CFE Software Bus 함수들 (Mock)
static inline void CFE_MSG_SetMsgId(CFE_MSG_Message_t *MsgPtr, CFE_SB_MsgId_t MsgId) {
    if (MsgPtr) {
        MsgPtr->Hdr.StreamId = (uint16_t)MsgId.Value;
    }
}

static inline void CFE_MSG_GetMsgId(CFE_SB_MsgId_t *MsgId, const CFE_MSG_Message_t *MsgPtr) {
    if (MsgId && MsgPtr) {
        MsgId->Value = MsgPtr->Hdr.StreamId;
    }
}

static inline void CFE_MSG_SetSequenceCount(CFE_MSG_Message_t *MsgPtr, uint16_t SeqCnt) {
    if (MsgPtr) {
        MsgPtr->Hdr.Sequence = SeqCnt;
    }
}

static inline void CFE_MSG_SetSegmentationFlag(CFE_MSG_Message_t *MsgPtr, uint16_t SegFlag) {
    if (MsgPtr) {
        MsgPtr->Hdr.Sequence = (MsgPtr->Hdr.Sequence & 0x3FFF) | ((SegFlag & 0x3) << 14);
    }
}

static inline void CFE_MSG_SetSize(CFE_MSG_Message_t *MsgPtr, uint16_t Size) {
    if (MsgPtr) {
        MsgPtr->Hdr.Length = Size;
    }
}

static inline void CFE_MSG_SetFcnCode(CFE_MSG_Message_t *MsgPtr, uint8_t FcnCode) {
    // Mock implementation - 실제로는 헤더 내부에 저장
    (void)MsgPtr;
    (void)FcnCode;
}

static inline void CFE_MSG_GetFcnCode(uint8_t *FcnCode, const CFE_MSG_Message_t *MsgPtr) {
    // Mock implementation
    if (FcnCode) {
        *FcnCode = 0; // 기본값
    }
    (void)MsgPtr;
}

static inline CFE_SB_MsgId_t CFE_SB_ValueToMsgId(uint32_t MsgIdValue) {
    CFE_SB_MsgId_t MsgId;
    MsgId.Value = MsgIdValue;
    return MsgId;
}

// CFE Event Services 함수들 (Mock)
static inline int32_t CFE_EVS_SendEvent(CFE_EVS_EventID_t EventID, 
                                       CFE_EVS_EventType_Enum_t EventType,
                                       const char *Spec, ...) {
    // Mock implementation - 실제로는 이벤트 메시지 전송
    (void)EventID;
    (void)EventType;
    (void)Spec;
    return CFE_SUCCESS;
}

// CFE Software Bus 함수들 (Mock)
static inline int32_t CFE_SB_SendMsg(CFE_MSG_Message_t *MsgPtr) {
    // Mock implementation - 실제로는 SB를 통해 메시지 전송
    (void)MsgPtr;
    return CFE_SUCCESS;
}

static inline int32_t CFE_SB_ReceiveBuffer(CFE_SB_Buffer_t **BufPtr, 
                                          CFE_SB_PipeId_t PipeId,
                                          int32_t TimeOut) {
    // Mock implementation - 실제로는 파이프에서 메시지 수신
    (void)BufPtr;
    (void)PipeId;
    (void)TimeOut;
    return CFE_SUCCESS;
}

// OSAL 함수들 (Mock)
static inline int32_t OS_SymbolLookup(uint64_t *SymbolAddress, const char *SymbolName) {
    // Mock implementation - 실제로는 심볼 테이블에서 주소 조회
    if (SymbolAddress && SymbolName) {
        // 시뮬레이션을 위한 더미 주소 반환
        *SymbolAddress = 0x20000000 + strlen(SymbolName) * 0x1000;
        return OS_SUCCESS;
    }
    return OS_INVALID_POINTER;
}

static inline int32_t OS_MemValidateRange(uint64_t Address, uint32_t Size, uint32_t MemoryType) {
    // Mock implementation - 메모리 범위 검증
    (void)Address;
    (void)Size;
    (void)MemoryType;
    return OS_SUCCESS; // 시뮬레이션에서는 모든 주소 허용
}

// 디버그 매크로
#ifdef CFS_DEBUG_MODE
    #define CFS_DEBUG_PRINT(fmt, ...) printf("[CFS_DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define CFS_DEBUG_PRINT(fmt, ...)
#endif

// CFS 환경 여부 확인
#ifdef CFS_ENVIRONMENT
    // 실제 CFS 환경에서는 실제 헤더 포함
    #include "cfe.h"
    #include "osapi.h"
    #include "cfe_sb.h"
    #include "cfe_evs.h"
    
    // Mock 정의들을 실제 CFS 정의로 덮어쓰기
    #undef CFE_MSG_SetMsgId
    #undef CFE_MSG_GetMsgId
    // ... 기타 함수들
#endif

#endif // CFS_CFE_MOCK_H