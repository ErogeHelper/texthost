#include "pch.h"
#include "host.h"
#include "texthost.h"
#include "extension.h"
#include "hookcode.h"
#include "yapi.h"
#include <fstream>

const wchar_t* ALREADY_INJECTED = L"Textractor: already injected";
const wchar_t* NEED_32_BIT = L"Textractor: architecture mismatch: only Textractor x86 can inject this process";
const wchar_t* NEED_64_BIT = L"Textractor: architecture mismatch: only Textractor x64 can inject this process";
const wchar_t* INVALID_CODEPAGE = L"Textractor: couldn't convert text (invalid codepage?)";
const wchar_t* INJECT_FAILED = L"Textractor: couldn't inject";
const wchar_t* INVALID_CODE = L"Textractor: invalid code";
const wchar_t* INVALID_PROCESS = L"Textractor: invalid process";
const wchar_t* INITIALIZED = L"Textractor: initialization completed";
const wchar_t* CONSOLE = L"Console";
const wchar_t* CLIPBOARD = L"Clipboard";

// const wchar_t* ALREADY_INJECTED = L"Textractor: �Ѿ�ע��";
// const wchar_t* NEED_32_BIT = L"Textractor: �ܹ���ƥ��: �볢��ʹ��32λ�汾��Textractor";
// const wchar_t* NEED_64_BIT = L"Textractor: �ܹ���ƥ��: �볢��ʹ��64λ�汾��Textractor";
// const wchar_t* INVALID_CODEPAGE = L"Textractor: �޷�ת���ı� (��Ч�Ĵ���ҳ?)";
// const wchar_t* INJECT_FAILED = L"Textractor: �޷�ע��";
// const wchar_t* INVALID_CODE = L"Textractor: ��Ч������";
// const wchar_t* INVALID_PROCESS = L"Textractor: ��Ч����ID";
// const wchar_t* INITIALIZED = L"Textractor: ��ʼ�����";
// const wchar_t* CONSOLE = L"����̨";
// const wchar_t* CLIPBOARD = L"������";

namespace TextHost
{
	DLLEXPORT BOOL WINAPI TextHostInit( ProcessEvent connect,
										ProcessEvent disconnect,
										OnCreateThread create,
										OnRemoveThread remove,
										ReceiveText output
									  )
	{
		auto createThread = [create](TextThread& thread)
		{
			create(thread.handle,
				thread.tp.processId,
				thread.tp.addr,
				thread.tp.ctx,
				thread.tp.ctx2,
				thread.name.c_str(),
				thread.hp.HookCode.c_str());
		};
		auto removeThread = [remove](TextThread& thread)
		{
			remove(thread.handle);
		};
		auto receiveText = [output](TextThread& thread, std::wstring& text)
		{
			if (thread.handle != 0)
			{
				Extension::RemoveRepeatChar(text);
				Extension::RemoveRepeatPhrase(text);
				text.erase(std::remove_if(text.begin(), text.end(), [](const wchar_t& c)
					{
						return c == L'\r' || c == L'\n'; 
					}), text.end());
				Extension::trim(text);
			}
			output(thread.handle, text.c_str(), text.size());
		};

		Host::Start(connect, disconnect, createThread, removeThread, receiveText);
		Host::AddConsoleOutput(INITIALIZED);
		return TRUE;
	}

	DLLEXPORT VOID WINAPI InjectProcess(DWORD processId)
	{
		Host::InjectProcess(processId);
	}

	DLLEXPORT VOID WINAPI DetachProcess(DWORD processId)
	{
		try { Host::DetachProcess(processId); }
		catch (std::out_of_range) { Host::AddConsoleOutput(INVALID_PROCESS); }
	}

	template <HookParam T>
	void InsertHook(DWORD processId, LPCWSTR command)
	{
		if (auto hp = HookCode::Parse<T>(command)) 
		{
			try
			{
				if constexpr (std::is_same<T, HookParamX86>::value)
					Host::InsertHookX86(processId, hp.value());
				else
					Host::InsertHookX64(processId, hp.value());
			}
			catch (std::out_of_range) { Host::AddConsoleOutput(INVALID_PROCESS); }
		}
		else { Host::AddConsoleOutput(INVALID_CODE); }
	}

	DLLEXPORT VOID WINAPI InsertHook(DWORD processId, LPCWSTR command)
	{
		if (AutoHandle<> hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId))
		{
			bool x64 = detail::Is64BitProcess(hProcess);
			x64 ? InsertHook<HookParamX64>(processId, command)
				: InsertHook<HookParamX86>(processId, command);
		}
	}

	DLLEXPORT VOID WINAPI RemoveHook(DWORD processId, uint64_t address)
	{
		try { Host::RemoveHook(processId, address); }
		catch (std::out_of_range){ Host::AddConsoleOutput(INVALID_PROCESS); }
	}

	template <SearchParam T>
	void SearchHookByText(DWORD processId, LPCWSTR text, int codepage = 932)
	{
		T sp = {};
		wcsncpy_s(sp.text, text, PATTERN_SIZE - 1);
		sp.codepage = codepage;
		try
		{
			if constexpr(std::is_same<T, SearchParamX86>::value)
				Host::FindHooksX86(processId, sp);
			else
				Host::FindHooksX64(processId, sp);
		}
		catch (std::out_of_range) { Host::AddConsoleOutput(INVALID_PROCESS); }
	}

	DLLEXPORT VOID WINAPI SearchForText(DWORD processId, LPCWSTR text, INT codepage)
	{
		if (AutoHandle<> hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId))
		{
			bool x64 = detail::Is64BitProcess(hProcess);
			x64 ? SearchHookByText<SearchParamX64>(processId, text, codepage)
				: SearchHookByText<SearchParamX86>(processId, text, codepage);
		}
	}

	DLLEXPORT VOID WINAPI SearchForHooks(DWORD processId, SearchParamX64* sp, ProcessEvent findHooks)
	{
		auto hooks = std::make_shared<std::vector<std::wstring>>();
		auto timeout = GetTickCount64() + sp->searchTime + 5000;

		try
		{
			if (AutoHandle<> hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId))
			{
				if (detail::Is64BitProcess(hProcess))
				{
					Host::FindHooksX64(processId, *sp, [hooks](HookParamX64 hp, std::wstring text)
						{
							hooks->push_back(HookCode::Generate(hp) + L" => " + text);
						});
				}
				else
				{
					SearchParamX86 sp_X86 = {};
					sp_X86.codepage = sp->codepage;
					sp_X86.length = sp->length;
					sp_X86.maxAddress = sp->maxAddress;
					sp_X86.minAddress = sp->minAddress;
					sp_X86.maxRecords = sp->maxRecords;
					sp_X86.offset = sp->offset;
					sp_X86.searchTime = sp->searchTime;
					sp_X86.padding = sp->padding;
					memcpy(sp_X86.pattern, sp->pattern, PATTERN_SIZE);
					wmemcpy(sp_X86.boundaryModule, sp->boundaryModule, MAX_MODULE_SIZE);
					wmemcpy(sp_X86.exportModule, sp->exportModule, MAX_MODULE_SIZE);

					Host::FindHooksX86(processId, sp_X86, [hooks](HookParamX86 hp, std::wstring text)
						{
							hooks->push_back(HookCode::Generate(hp) + L" => " + text);
						});
				}
			}
			
		}
		catch (std::out_of_range)
		{
			Host::AddConsoleOutput(INVALID_PROCESS);
			return;
		}

		std::thread([hooks, processId, timeout, findHooks]
			{
				for (int lastSize = 0; hooks->size() == 0 || hooks->size() != lastSize; Sleep(2000))
				{
					lastSize = hooks->size();
					if (GetTickCount64() > timeout) break; 
				}
				static std::string location = std::filesystem::current_path().string() + "\\";
				std::ofstream saveFile(location + "result.txt");
				if (saveFile.is_open())
				{
					for (std::vector<std::wstring>::const_iterator it = hooks->begin(); it != hooks->end(); ++it)
					{
						saveFile << WideStringToString(*it) << std::endl; //utf-8
					}
					saveFile.close();
					if (hooks->size() != 0 && findHooks != nullptr)
						findHooks(processId);
				}
				hooks->clear();
			}).detach();
	}

	DLLEXPORT VOID WINAPI AddClipboardThread(HWND handle)
	{	
		if (AddClipboardFormatListener(handle))
		{
			auto threadId = GetWindowThreadProcessId(handle, NULL);
			Host::AddClipboardThread(threadId);
		}
	}
}

