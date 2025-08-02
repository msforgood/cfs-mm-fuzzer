/*
 * MM LoadMemFromFile 명령을 위한 퍼징 하니스
 * CFS(Core Flight System) MM 모듈의 파일에서 메모리로 로드하는 명령어 처리 함수
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// CFS 공통 헤더들
#include "../../common/cfe_mock.h"
#include "../../common/cfs_config.h"
#include "../../common/mm_types.h"

// 하니스 설정
#define MM_LOAD_HARNESS_VERSION "1.0.0"

// 안전한 메모리 조작 매크로들
#define CFS_SAFE_MEMCPY(dst, src, size) do { \
    if ((dst) && (src) && (size) > 0) { \
        memcpy(dst, src, size); \
    } \
} while(0)

#define CFS_SAFE_STRNCPY(dst, src, size) do { \
    if ((dst) && (src) && (size) > 0) { \
        strncpy(dst, src, size - 1); \
        ((char*)(dst))[(size) - 1] = '\0'; \
    } \
} while(0)

#define CFS_SAFE_STRNCAT(dst, src, size) do { \
    if ((dst) && (src) && (size) > 0) { \
        strncat(dst, src, (size) - strlen(dst) - 1); \
    } \
} while(0)

// 디버그 매크로들
#ifdef CFS_DEBUG_MODE
    #define CFS_DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
    #define CFS_INFO_PRINT(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
    #define CFS_ERROR_PRINT(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
    #define CFS_DEBUG_PRINT(fmt, ...)
    #define CFS_INFO_PRINT(fmt, ...)
    #define CFS_ERROR_PRINT(fmt, ...)
#endif

/*
 * MM_LoadMemFromFileCmd 패킷 생성 함수
 * 
 * @param Data: 퍼징 입력 데이터
 * @param Size: 입력 데이터 크기
 * @return: 생성된 패킷 포인터 (호출자가 해제 책임)
 */
void* MM_Load_ConstructPacket(const uint8_t *Data, size_t Size) {
    const size_t PACKET_SIZE = sizeof(MM_LoadMemFromFileCmd_t);
    
    if (Size < PACKET_SIZE) {
        return NULL;
    }
    
    MM_LoadMemFromFileCmd_t *packet = malloc(PACKET_SIZE);
    if (!packet) {
        return NULL;
    }
    
    memset(packet, 0, PACKET_SIZE);
    
    // 기본 헤더 설정
    CFE_SB_MsgId_t msg_id = CFE_SB_ValueToMsgId(MM_CMD_MID);
    CFE_MSG_SetMsgId((CFE_MSG_Message_t*)&packet->CmdHeader, msg_id);
    CFE_MSG_SetSize((CFE_MSG_Message_t*)&packet->CmdHeader, PACKET_SIZE);
    CFE_MSG_SetSequenceCount((CFE_MSG_Message_t*)&packet->CmdHeader, 0);
    CFE_MSG_SetSegmentationFlag((CFE_MSG_Message_t*)&packet->CmdHeader, 0x3);
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t*)&packet->CmdHeader, MM_LOAD_MEM_FROM_FILE_CC);
    
    // 퍼징 데이터 기반 필드 설정
    size_t data_offset = 0;
    
    // MemType 설정 (1=RAM, 2=EEPROM)
    if (data_offset < Size) {
        packet->MemType = (Data[data_offset] % 2) + 1;
        data_offset++;
    } else {
        packet->MemType = MM_RAM; // 기본값: RAM
    }
    
    // NumOfBytes 설정 (메모리 타입에 따른 최대값 제한)
    uint32_t max_bytes = (packet->MemType == MM_RAM) ? 
                        MM_MAX_LOAD_FILE_DATA_RAM : MM_MAX_LOAD_FILE_DATA_EEPROM;
    
    if (data_offset + sizeof(uint32_t) <= Size) {
        CFS_SAFE_MEMCPY(&packet->NumOfBytes, Data + data_offset, sizeof(uint32_t));
        packet->NumOfBytes = (packet->NumOfBytes % max_bytes) + 1;
        data_offset += sizeof(uint32_t);
    } else {
        packet->NumOfBytes = 1024; // 기본값
    }
    
    // CRC 설정
    if (data_offset + sizeof(uint32_t) <= Size) {
        CFS_SAFE_MEMCPY(&packet->CRC, Data + data_offset, sizeof(uint32_t));
        data_offset += sizeof(uint32_t);
    } else {
        packet->CRC = 0; // CRC 사용 안함
    }
    
    // 목적지 주소 설정
    if (data_offset < Size) {
        uint8_t choice_byte = Data[data_offset];
        data_offset++;
        
        if (choice_byte % 2 == 0) {
            // 심볼 이름 사용
            if (CFS_NUM_VALID_SYMBOLS > 0 && data_offset < Size) {
                size_t symbol_idx = Data[data_offset] % CFS_NUM_VALID_SYMBOLS;
                CFS_SAFE_STRNCPY(packet->DestSymAddress.SymName, 
                               cfs_valid_symbols[symbol_idx], 
                               OS_MAX_API_NAME);
                packet->DestSymAddress.Offset = 0;
                data_offset++;
            }
        } else {
            // 오프셋 주소 사용
            packet->DestSymAddress.SymName[0] = '\0';
            if (data_offset + sizeof(uint64_t) <= Size) {
                CFS_SAFE_MEMCPY(&packet->DestSymAddress.Offset, Data + data_offset, sizeof(uint64_t));
                // 메모리 주소를 타겟별 기본 주소 기반으로 제한
                uint64_t base_addr = (packet->MemType == MM_RAM) ? 
                                   CFS_DEFAULT_RAM_ADDR : CFS_DEFAULT_EEPROM_ADDR;
                packet->DestSymAddress.Offset = base_addr + (packet->DestSymAddress.Offset & 0x7FFFFFFF);
                data_offset += sizeof(uint64_t);
            } else {
                packet->DestSymAddress.Offset = (packet->MemType == MM_RAM) ? 
                                              CFS_DEFAULT_RAM_ADDR : CFS_DEFAULT_EEPROM_ADDR;
            }
        }
    }
    
    // 파일 이름 설정
    if (data_offset < Size) {
        uint8_t path_choice = Data[data_offset] % CFS_NUM_VALID_PATHS;
        CFS_SAFE_STRNCPY(packet->FileName, cfs_valid_paths[path_choice], OS_MAX_PATH_LEN);
        data_offset++;
        
        // 파일 이름 추가
        const char* filename_suffix = "testfile.bin";
        size_t current_len = strlen(packet->FileName);
        size_t remaining = OS_MAX_PATH_LEN - current_len - 1;
        
        if (remaining > strlen(filename_suffix)) {
            CFS_SAFE_STRNCAT(packet->FileName, filename_suffix, OS_MAX_PATH_LEN);
        }
    } else {
        CFS_SAFE_STRNCPY(packet->FileName, "/cf/apps/default.bin", OS_MAX_PATH_LEN);
    }
    
    CFS_DEBUG_PRINT("Constructed packet: MemType=%d, NumOfBytes=%u, File=%s", 
                     packet->MemType, packet->NumOfBytes, packet->FileName);
    
    return packet;
}

/*
 * MM_LoadMemFromFileCmd 함수 시뮬레이션
 * 실제 CFS 환경에서는 mm_load.c의 MM_LoadMemFromFileCmd 함수를 호출
 */
int32_t MM_LoadMemFromFileCmd_Simulation(MM_LoadMemFromFileCmd_t *CmdPtr) {
    if (!CmdPtr) {
        CFS_ERROR_PRINT("Null command pointer");
        return OS_INVALID_POINTER;
    }
    
    CFS_DEBUG_PRINT("Processing MM Load command");
    
    // 메모리 타입 검증
    if (CmdPtr->MemType != MM_RAM && CmdPtr->MemType != MM_EEPROM) {
        CFS_ERROR_PRINT("Invalid memory type: %d", CmdPtr->MemType);
        return CFS_MM_ERROR_INVALID_MEMTYPE;
    }
    
    // 데이터 크기 검증
    if (CmdPtr->NumOfBytes == 0) {
        CFS_ERROR_PRINT("Invalid data size: %u", CmdPtr->NumOfBytes);
        return CFS_MM_ERROR_INVALID_SIZE;
    }
    
    uint32_t max_size = (CmdPtr->MemType == MM_RAM) ? 
                       MM_MAX_LOAD_FILE_DATA_RAM : MM_MAX_LOAD_FILE_DATA_EEPROM;
    if (CmdPtr->NumOfBytes > max_size) {
        CFS_ERROR_PRINT("Data size exceeds limit: %u > %u", CmdPtr->NumOfBytes, max_size);
        return CFS_MM_ERROR_INVALID_SIZE;
    }
    
    // 파일 이름 검증
    if (strlen(CmdPtr->FileName) == 0) {
        CFS_ERROR_PRINT("Empty filename");
        return CFS_MM_ERROR_INVALID_FILE;
    }
    
    // 경로 순회 공격 방지
    if (strstr(CmdPtr->FileName, "../") || strstr(CmdPtr->FileName, "..\\")) {
        CFS_ERROR_PRINT("Path traversal detected in filename: %s", CmdPtr->FileName);
        return CFS_MM_ERROR_INVALID_FILE;
    }
    
    // 심볼 주소 해석
    uint64_t resolved_addr = 0;
    if (strlen(CmdPtr->DestSymAddress.SymName) > 0) {
        // 심볼 이름이 있는 경우 - Mock에서는 간단히 계산
        resolved_addr = CFS_DEFAULT_RAM_ADDR + strlen(CmdPtr->DestSymAddress.SymName) * 0x1000;
        resolved_addr += CmdPtr->DestSymAddress.Offset;
        CFS_DEBUG_PRINT("Using symbol: %s with offset: 0x%lx", 
                       CmdPtr->DestSymAddress.SymName, CmdPtr->DestSymAddress.Offset);
    } else {
        // 직접 주소 사용
        resolved_addr = CmdPtr->DestSymAddress.Offset;
        CFS_DEBUG_PRINT("Using direct address: 0x%lx", resolved_addr);
    }
    
    // 메모리 주소 검증
    if (!CFS_IsValidMemoryAddress(resolved_addr, CmdPtr->MemType, CmdPtr->NumOfBytes)) {
        CFS_ERROR_PRINT("Invalid memory region: addr=0x%08lX, size=%u", 
                        resolved_addr, CmdPtr->NumOfBytes);
        return CFS_MM_ERROR_INVALID_ADDR;
    }
    
    // 메모리 정렬 검증
    if ((resolved_addr % CFS_MEMORY_ALIGNMENT) != 0) {
        CFS_ERROR_PRINT("Address not properly aligned: 0x%08lX", resolved_addr);
        return CFS_MM_ERROR_ALIGNMENT;
    }
    
    CFS_INFO_PRINT("MM Load simulation successful: %u bytes from %s to 0x%08lX", 
                   CmdPtr->NumOfBytes, CmdPtr->FileName, resolved_addr);
    
    // 실제 환경에서는 여기서 파일을 읽어 메모리에 로드
    #ifndef CFS_FUZZING_MODE
        #ifdef CFS_ENVIRONMENT
            // 실제 MM 함수 호출
            // return MM_LoadMemFromFileCmd((CFE_SB_MsgPtr_t)CmdPtr);
        #endif
    #endif
    
    return CFE_SUCCESS;
}

/*
 * LibFuzzer 엔트리 포인트
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // 최소 패킷 크기 확인
    if (size < sizeof(MM_LoadMemFromFileCmd_t)) {
        return 0;
    }
    
    // 패킷 생성
    MM_LoadMemFromFileCmd_t *packet = (MM_LoadMemFromFileCmd_t*)MM_Load_ConstructPacket(data, size);
    if (!packet) {
        return 0;
    }
    
    // 함수 테스트
    int32_t status = MM_LoadMemFromFileCmd_Simulation(packet);
    (void)status; // 사용되지 않는 변수 경고 방지
    
    // 메모리 해제
    free(packet);
    
    return 0;
}

/*
 * 초기화 함수 (LibFuzzer에서 호출)
 */
int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc;
    (void)argv;
    
    printf("CFS MM Load Harness %s initialized\n", MM_LOAD_HARNESS_VERSION);
    printf("Target: MM_LoadMemFromFileCmd\n");
    printf("Packet size: %zu bytes\n", sizeof(MM_LoadMemFromFileCmd_t));
    
    #ifdef CFS_DEBUG_MODE
        printf("Debug mode enabled\n");
    #endif
    
    return 0;
}

/*
 * 테스트용 메인 함수 (퍼저 없이 단독 테스트시 사용)
 */
#ifdef STANDALONE_TEST
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("=== CFS MM Load Harness Test ===\n");
    printf("Version: %s\n", MM_LOAD_HARNESS_VERSION);
    
    // 테스트 데이터 생성
    uint8_t test_data[] = {
        0x01,                    // MemType choice
        0x00, 0x10, 0x00, 0x00, // NumOfBytes (4096)
        0x12, 0x34, 0x56, 0x78, // CRC
        0x00,                    // Symbol choice (even = use symbol)
        0x02,                    // Symbol index
        0x01                     // File path choice
    };
    
    printf("Test data size: %zu bytes\n", sizeof(test_data));
    
    MM_LoadMemFromFileCmd_t *packet = (MM_LoadMemFromFileCmd_t*)MM_Load_ConstructPacket(test_data, sizeof(test_data));
    if (packet) {
        printf("Packet created successfully:\n");
        printf("  MemType: %u\n", packet->MemType);
        printf("  NumOfBytes: %u\n", packet->NumOfBytes);
        printf("  CRC: 0x%08X\n", packet->CRC);
        printf("  Symbol: %s\n", packet->DestSymAddress.SymName);
        printf("  Offset: 0x%lx\n", packet->DestSymAddress.Offset);
        printf("  FileName: %s\n", packet->FileName);
        
        printf("\nTesting MM_LoadMemFromFileCmd simulation...\n");
        int32_t result = MM_LoadMemFromFileCmd_Simulation(packet);
        printf("Result: %d (%s)\n", result, (result == CFE_SUCCESS) ? "SUCCESS" : "ERROR");
        
        // 여러 번 실행하여 다양한 케이스 테스트
        printf("\nRunning multiple iterations...\n");
        for (int i = 0; i < 10; i++) {
            // 약간씩 다른 데이터로 테스트
            uint8_t modified_data[sizeof(test_data)];
            memcpy(modified_data, test_data, sizeof(test_data));
            modified_data[0] = (i % 2) + 1; // MemType 변경
            
            MM_LoadMemFromFileCmd_t *test_packet = (MM_LoadMemFromFileCmd_t*)MM_Load_ConstructPacket(modified_data, sizeof(modified_data));
            if (test_packet) {
                int32_t test_result = MM_LoadMemFromFileCmd_Simulation(test_packet);
                printf("  Iteration %d: %s\n", i+1, (test_result == CFE_SUCCESS) ? "PASS" : "FAIL");
                free(test_packet);
            }
        }
        
        free(packet);
        printf("\nTest completed successfully!\n");
    } else {
        printf("Failed to create packet\n");
        return 1;
    }
    
    return 0;
}
#endif