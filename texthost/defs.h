#pragma once

// texthook/defs.h
// 8/23/2013 jichi

// Pipes

constexpr auto HOOK_PIPE = L"\\\\.\\pipe\\TEXTRACTOR_HOOK";
constexpr auto HOST_PIPE = L"\\\\.\\pipe\\TEXTRACTOR_HOST";

// Sections

constexpr auto ITH_SECTION_ = L"VNR_SECTION_"; // _%d

// Mutexes

constexpr auto ITH_HOOKMAN_MUTEX_ = L"VNR_HOOKMAN_"; // ITH_HOOKMAN_%d
constexpr auto CONNECTING_MUTEX = L"TEXTRACTOR_CONNECTING_PIPES";

// Events

constexpr auto PIPE_AVAILABLE_EVENT = L"TEXTRACTOR_PIPE_AVAILABLE";

// Files (depend on Root dir)

constexpr auto INJECTOR_EXE_X86 = L"libs\\x86\\injector.exe";
constexpr auto INJECTOR_EXE_X64 = L"libs\\x64\\injector.exe";

constexpr auto ITH_DLL_X86 = L"libs\\x86\\texthook.dll"; 
constexpr auto ITH_DLL_X64 = L"libs\\x64\\texthook.dll"; 

constexpr auto& GAME_CONFIG_FILE = L"TextractorConfig.txt";

// EOF
