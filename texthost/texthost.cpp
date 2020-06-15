#include "pch.h"
#include "host.h"
#include "texthost.h"
#include "extension.h"
#include "hookcode.h"
#include <fstream>

//const wchar_t* ALREADY_INJECTED = L"Textractor: already injected";
//const wchar_t* NEED_32_BIT = L"Textractor: architecture mismatch: only Textractor x86 can inject this process";
//const wchar_t* NEED_64_BIT = L"Textractor: architecture mismatch: only Textractor x64 can inject this process";
//const wchar_t* INVALID_CODEPAGE = L"Textractor: couldn't convert text (invalid codepage?)";
//const wchar_t* INJECT_FAILED = L"Textractor: couldn't inject";
//const wchar_t* INVALID_CODE = L"Textractor: invalid code";
//const wchar_t* INVALID_PROCESS = L"Textractor: invalid process";
//const wchar_t* INITIALIZED = L"Textractor: initialization completed";
//const wchar_t* CONSOLE = L"Console";
//const wchar_t* CLIPBOARD = L"Clipboard";

const wchar_t* ALREADY_INJECTED = L"Textractor: �Ѿ�ע��";
const wchar_t* NEED_32_BIT = L"Textractor: �ܹ���ƥ��: �볢��ʹ��32λ�汾��Textractor";
const wchar_t* NEED_64_BIT = L"Textractor: �ܹ���ƥ��: �볢��ʹ��64λ�汾��Textractor";
const wchar_t* INVALID_CODEPAGE = L"Textractor: �޷�ת���ı� (��Ч�Ĵ���ҳ?)";
const wchar_t* INJECT_FAILED = L"Textractor: �޷�ע��";
const wchar_t* INVALID_CODE = L"Textractor: ��Ч������";
const wchar_t* INVALID_PROCESS = L"Textractor: ��Ч����ID";
const wchar_t* INITIALIZED = L"Textractor: ��ʼ�����";
const wchar_t* CONSOLE = L"����̨";
const wchar_t* CLIPBOARD = L"������";

namespace TextHost
{
	DLLEXPORT BOOL WINAPI TextHostInit( ProcessEvent connect,
										ProcessEvent disconnect,
										OnCreateThread create,
										OnRemoveThread remove,
										OutputText output
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
				HookCode::Generate(thread.hp, thread.tp.processId).c_str());
		};
		auto removeThread = [remove](TextThread& thread)
		{
			remove(thread.handle);
		};
		auto outputText = [output](TextThread& thread, std::wstring& text)
		{
			if (thread.handle != 0)
			{
				Extension::RemoveRepeatChar(text);
				Extension::RemoveRepeatPhrase(text);
				text.erase(std::remove_if(text.begin(), text.end(), [](const wchar_t& c)
					{
						return c == L'\r' || c == L'\n';
					}), text.end());
			}
			output(thread.handle, text.c_str());
		};

		Host::Start(connect, disconnect, createThread, removeThread, outputText);
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

	DLLEXPORT VOID WINAPI InsertHook(DWORD processId, LPCWSTR command)
	{
		if (auto hp = HookCode::Parse(command))
			try { Host::InsertHook(processId, hp.value()); }
		catch (std::out_of_range){ Host::AddConsoleOutput(INVALID_PROCESS); }
		else { Host::AddConsoleOutput(INVALID_CODE); }
	}

	DLLEXPORT VOID WINAPI RemoveHook(DWORD processId, uint64_t address)
	{
		try { Host::RemoveHook(processId, address); }
		catch (std::out_of_range)
		{
			Host::AddConsoleOutput(INVALID_PROCESS);
		}
	}

	DLLEXPORT VOID WINAPI SearchForText(DWORD processId, LPCWSTR text, INT codepage)
	{
		SearchParam sp = {};
		wcsncpy_s(sp.text, text, PATTERN_SIZE - 1);
		sp.codepage = codepage;
		try { Host::FindHooks(processId, sp); }
		catch (std::out_of_range) { Host::AddConsoleOutput(INVALID_PROCESS); }
		catch (std::exception ex) { Host::AddConsoleOutput(StringToWideString(ex.what())); }
	}

	DLLEXPORT VOID WINAPI SearchForHooks(DWORD processId, SearchParam* sp, FindHooks findHooks)
	{
		auto hooks = std::make_shared<std::vector<std::wstring>>();
		auto timeout = GetTickCount64() + sp->searchTime + 5000;

		try
		{
			Host::FindHooks(processId, *sp, [hooks](HookParam hp, std::wstring text)
				{
					hooks->push_back(HookCode::Generate(hp) + L" => " + text);
				});
		}
		catch (std::out_of_range)
		{
			Host::AddConsoleOutput(INVALID_PROCESS);
			return;
		}

		std::thread([hooks, timeout, findHooks]
			{
				for (int lastSize = 0; hooks->size() == 0 || hooks->size() != lastSize; Sleep(2000))
				{
					lastSize = hooks->size();
					if (GetTickCount64() > timeout) break; //���û���ҵ������sizeʼ��Ϊ0����������ѭ���������趨��ʱʱ��
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
					if (hooks->size() != 0) findHooks();
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

