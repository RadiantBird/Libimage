#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstring>  // memset用

// 再生用ユーザーデータ構造体
struct AudioUserData {
    ma_decoder* decoder;
    std::atomic<bool>* isPlaying;
};

// miniaudioのエラーコードを文字列に変換
const char* ma_result_to_string(ma_result result) {
    switch (result) {
        case MA_SUCCESS: return "MA_SUCCESS";
        case MA_ERROR: return "MA_ERROR";
        case MA_INVALID_ARGS: return "MA_INVALID_ARGS";
        case MA_INVALID_OPERATION: return "MA_INVALID_OPERATION";
        case MA_OUT_OF_MEMORY: return "MA_OUT_OF_MEMORY";
        case MA_OUT_OF_RANGE: return "MA_OUT_OF_RANGE";
        case MA_ACCESS_DENIED: return "MA_ACCESS_DENIED";
        case MA_DOES_NOT_EXIST: return "MA_DOES_NOT_EXIST";
        case MA_ALREADY_EXISTS: return "MA_ALREADY_EXISTS";
        case MA_TOO_MANY_OPEN_FILES: return "MA_TOO_MANY_OPEN_FILES";
        case MA_INVALID_FILE: return "MA_INVALID_FILE";
        case MA_TOO_BIG: return "MA_TOO_BIG";
        case MA_PATH_TOO_LONG: return "MA_PATH_TOO_LONG";
        case MA_NAME_TOO_LONG: return "MA_NAME_TOO_LONG";
        case MA_NOT_DIRECTORY: return "MA_NOT_DIRECTORY";
        case MA_IS_DIRECTORY: return "MA_IS_DIRECTORY";
        case MA_DIRECTORY_NOT_EMPTY: return "MA_DIRECTORY_NOT_EMPTY";
        case MA_AT_END: return "MA_AT_END";
        case MA_NO_SPACE: return "MA_NO_SPACE";
        case MA_BUSY: return "MA_BUSY";
        case MA_IO_ERROR: return "MA_IO_ERROR";
        case MA_INTERRUPT: return "MA_INTERRUPT";
        case MA_UNAVAILABLE: return "MA_UNAVAILABLE";
        case MA_ALREADY_IN_USE: return "MA_ALREADY_IN_USE";
        case MA_BAD_ADDRESS: return "MA_BAD_ADDRESS";
        case MA_BAD_SEEK: return "MA_BAD_SEEK";
        case MA_BAD_PIPE: return "MA_BAD_PIPE";
        case MA_DEADLOCK: return "MA_DEADLOCK";
        case MA_TOO_MANY_LINKS: return "MA_TOO_MANY_LINKS";
        case MA_NOT_IMPLEMENTED: return "MA_NOT_IMPLEMENTED";
        case MA_NO_MESSAGE: return "MA_NO_MESSAGE";
        case MA_BAD_MESSAGE: return "MA_BAD_MESSAGE";
        case MA_NO_DATA_AVAILABLE: return "MA_NO_DATA_AVAILABLE";
        case MA_INVALID_DATA: return "MA_INVALID_DATA";
        case MA_TIMEOUT: return "MA_TIMEOUT";
        case MA_NO_NETWORK: return "MA_NO_NETWORK";
        case MA_NOT_UNIQUE: return "MA_NOT_UNIQUE";
        case MA_NOT_SOCKET: return "MA_NOT_SOCKET";
        case MA_NO_ADDRESS: return "MA_NO_ADDRESS";
        case MA_BAD_PROTOCOL: return "MA_BAD_PROTOCOL";
        case MA_PROTOCOL_UNAVAILABLE: return "MA_PROTOCOL_UNAVAILABLE";
        case MA_PROTOCOL_NOT_SUPPORTED: return "MA_PROTOCOL_NOT_SUPPORTED";
        case MA_PROTOCOL_FAMILY_NOT_SUPPORTED: return "MA_PROTOCOL_FAMILY_NOT_SUPPORTED";
        case MA_ADDRESS_FAMILY_NOT_SUPPORTED: return "MA_ADDRESS_FAMILY_NOT_SUPPORTED";
        case MA_SOCKET_NOT_SUPPORTED: return "MA_SOCKET_NOT_SUPPORTED";
        case MA_CONNECTION_RESET: return "MA_CONNECTION_RESET";
        case MA_ALREADY_CONNECTED: return "MA_ALREADY_CONNECTED";
        case MA_NOT_CONNECTED: return "MA_NOT_CONNECTED";
        case MA_CONNECTION_REFUSED: return "MA_CONNECTION_REFUSED";
        case MA_NO_HOST: return "MA_NO_HOST";
        case MA_IN_PROGRESS: return "MA_IN_PROGRESS";
        case MA_CANCELLED: return "MA_CANCELLED";
        case MA_MEMORY_ALREADY_MAPPED: return "MA_MEMORY_ALREADY_MAPPED";
        default: return "UNKNOWN_ERROR";
    }
}

int main() {
    std::cout << "=== miniaudio Audio Player Test ===" << std::endl;
    
    // --- 1. ファイルパス ---
    std::string path = "assets/audio/1991.wav";  // 相対パスを修正
    std::cout << "Attempting to load: " << path << std::endl;
    
    // ファイル存在確認（追加デバッグ）
    FILE* testFile = fopen(path.c_str(), "rb");
    if (testFile) {
        fseek(testFile, 0, SEEK_END);
        long fileSize = ftell(testFile);
        fclose(testFile);
        std::cout << "✓ File exists, size: " << fileSize << " bytes" << std::endl;
    } else {
        std::cerr << "✗ File does not exist or cannot be opened!" << std::endl;
        std::cerr << "  Tried path: " << path << std::endl;
        std::cerr << "  Try: assets/audio/1991.wav" << std::endl;
        return -1;
    }

    ma_result result;
    ma_decoder decoder;
    ma_device device;
    std::atomic<bool> isPlaying(true);

    // --- 2. デコーダ初期化 ---
    std::cout << "\nInitializing decoder..." << std::endl;
    result = ma_decoder_init_file(path.c_str(), nullptr, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "✗ Failed to initialize decoder!" << std::endl;
        std::cerr << "  Error code: " << result << " (" << ma_result_to_string(result) << ")" << std::endl;
        return -1;
    }
    std::cout << "✓ Decoder initialized successfully" << std::endl;
    std::cout << "  Format: " << decoder.outputFormat << std::endl;
    std::cout << "  Channels: " << decoder.outputChannels << std::endl;
    std::cout << "  Sample Rate: " << decoder.outputSampleRate << " Hz" << std::endl;

    // --- 3. 再生デバイス初期化 ---
    std::cout << "\nInitializing playback device..." << std::endl;
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;

    AudioUserData userData { &decoder, &isPlaying };
    deviceConfig.pUserData = &userData;

    // データコールバック
    deviceConfig.dataCallback = [](ma_device* pDevice, void* pOutput, const void*, ma_uint32 frameCount) {
        AudioUserData* ud = (AudioUserData*)pDevice->pUserData;
        
        ma_uint64 framesRead = 0;
        ma_result readResult = ma_decoder_read_pcm_frames(ud->decoder, pOutput, frameCount, &framesRead);
        
        if (readResult != MA_SUCCESS && readResult != MA_AT_END) {
            std::cerr << "Error reading frames: " << readResult << std::endl;
        }

        if (framesRead < frameCount) {
            // 残りは 0 埋め
            size_t bytesPerSample = ma_get_bytes_per_sample(ud->decoder->outputFormat);
            size_t remainingBytes = (frameCount - (ma_uint32)framesRead) * ud->decoder->outputChannels * bytesPerSample;
            memset((char*)pOutput + framesRead * ud->decoder->outputChannels * bytesPerSample, 0, remainingBytes);
            
            if (framesRead == 0) {
                *(ud->isPlaying) = false;
            }
        }
    };

    result = ma_device_init(nullptr, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        std::cerr << "✗ Failed to initialize playback device!" << std::endl;
        std::cerr << "  Error code: " << result << " (" << ma_result_to_string(result) << ")" << std::endl;
        ma_decoder_uninit(&decoder);
        return -1;
    }
    std::cout << "✓ Playback device initialized" << std::endl;
    std::cout << "  Device name: " << device.playback.name << std::endl;

    // --- 4. 再生開始 ---
    std::cout << "\nStarting playback..." << std::endl;
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        std::cerr << "✗ Failed to start playback device!" << std::endl;
        std::cerr << "  Error code: " << result << " (" << ma_result_to_string(result) << ")" << std::endl;
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -1;
    }

    std::cout << "✓ Playing " << path << "..." << std::endl;
    std::cout << "  (Press Ctrl+C to stop)" << std::endl;

    // --- 5. 再生待機 ---
    int seconds = 0;
    while (isPlaying.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (++seconds % 10 == 0) {
            std::cout << "  Playing... (" << (seconds / 10) << "s)" << std::endl;
        }
    }

    std::cout << "\n✓ Playback finished." << std::endl;

    // --- 6. クリーンアップ ---
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

    std::cout << "=== Test Complete ===" << std::endl;
    return 0;
}