#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>

struct Proc
{
    DWORD pid;
    std::wstring name;
};

bool ContainsI(const std::wstring& s, const std::wstring& sub)
{
    return _wcsicmp(s.c_str(), sub.c_str()) == 0 ||
        (s.size() >= sub.size() &&
            _wcsicmp(s.c_str(), sub.c_str()) == 0) ||
        (s.find(sub) != std::wstring::npos);
}

bool IsDiscordStable(const std::wstring& name)
{
    return _wcsicmp(name.c_str(), L"Discord.exe") == 0;
}

bool IsDiscordPTB(const std::wstring& name)
{
    return _wcsicmp(name.c_str(), L"DiscordPTB.exe") == 0;
}

bool IsDiscordCanary(const std::wstring& name)
{
    return _wcsicmp(name.c_str(), L"DiscordCanary.exe") == 0;
}

bool IsDiscordUpdater(const std::wstring& name)
{
    return _wcsicmp(name.c_str(), L"Update.exe") == 0;
}

std::vector<Proc> GetDiscordProcesses(bool stable, bool ptb, bool canary)
{
    std::vector<Proc> list;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return list;

    PROCESSENTRY32W pe{ sizeof(pe) };

    if (Process32FirstW(snap, &pe))
    {
        do
        {
            std::wstring exe = pe.szExeFile;

            bool match =
                (stable && IsDiscordStable(exe)) ||
                (ptb && IsDiscordPTB(exe)) ||
                (canary && IsDiscordCanary(exe)) ||
                ((ptb || canary || stable) && IsDiscordUpdater(exe));

            if (match)
            {
                list.push_back({ pe.th32ProcessID, exe });
            }

        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
    return list;
}

void KillProcesses(const std::vector<Proc>& list)
{
    for (const auto& p : list)
    {
        HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, p.pid);
        if (h)
        {
            TerminateProcess(h, 0);
            CloseHandle(h);
        }
    }
}

void ShowMenu()
{
    std::cout <<
        
        "        Discord Process Terminator    \n"
        
        "        1. Kill Discord (Stable)\n"
        "        2. Kill Discord PTB\n"
        "        3. Kill Discord Canary\n"
        "        4. Kill  ALL Discord Versions\n"
        "        0. Exit\n"
        " Select option: ";
}

int main()
{
    int choice;
    ShowMenu();
    std::cin >> choice;

    bool stable = false, ptb = false, canary = false;

    switch (choice)
    {
    case 1: stable = true; break;
    case 2: ptb = true; break;
    case 3: canary = true; break;
    case 4: stable = ptb = canary = true; break;
    default: return 0;
    }

    auto list = GetDiscordProcesses(stable, ptb, canary);

    if (list.empty())
        std::cout << "\nNo selected Discord version running.\n";
    else
    {
        KillProcesses(list);
        std::cout << "\nDiscord terminated successfully.\n";
    }

    system("pause");
    return 0;
}
