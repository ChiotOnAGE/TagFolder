// DesktopINI.cpp : 定义应用程序的入口点。
//

#include <string>
#include <Windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include "DesktopINI.h"
#include "SimpleIni.h"

#pragma comment(lib, "Shlwapi.lib")

std::string convert_wchar_to_utf8(const WCHAR* str)
{
    std::string result;
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    result.resize(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, &result[0], size_needed, NULL, NULL);
    return result;
}

INT_PTR CALLBACK TagEditDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static WCHAR* result;
    switch (uMsg) {
        case WM_INITDIALOG:
            result = (WCHAR*)lParam;
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    // 获取用户输入的文本
                    GetDlgItemText(hwndDlg, IDC_INPUTBOX, result, sizeof(result));

                    // 关闭对话框
                    EndDialog(hwndDlg, 0);
                    PostQuitMessage(0);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}

int APIENTRY wWinMain(
    _In_ HINSTANCE      hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ LPWSTR         lpCmdLine,
    _In_ int            nCmdShow
) {
    //获取命令行参数
    extern int __argc;
    extern wchar_t** __wargv;

    //判断 __wargv 是有且仅有一个参数
    if (!(__argc == 2)) {
		return -1;
	}

    // 将 __wargv[1] 赋值给 directoryName
    wchar_t* directoryName = __wargv[1];

    //判断 directoryName 是否存在并为文件夹
    if (!(PathFileExistsW(directoryName) && PathIsDirectoryW(directoryName))) {
		return -1;
	}

    //判断 desktop.ini 是否存在
    WCHAR desktopINIPath[MAX_PATH];
    PathCombineW(desktopINIPath, directoryName, L"desktop.ini");
    if (!PathFileExistsW(desktopINIPath)) {
		// 如果 desktop.ini 不存在, 则创建空白的 desktop.ini
        HANDLE hFile = CreateFileW(desktopINIPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) { return -1; } else { CloseHandle(hFile); }
	}

    //使用 SimpleIni 库读取 desktop.ini
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error err = ini.LoadFile(desktopINIPath);
    if (err != SI_OK) { return -1; }

    //获取 [{F29F85E0-4FF9-1068-AB91-08002B27B3D9}] 节中 Prop5 的值，若不存在赋值为空
    std::string rawTags = ini.GetValue("{F29F85E0-4FF9-1068-AB91-08002B27B3D9}", "Prop5", "");
    //若 tags 不为空，判断 tags 是否以 "31," 开头，若不是则在 tags 前加上 "31,"
    if (rawTags.find("31,") != 0) {
        rawTags = "31," + rawTags;
	}
    //将去除 '31,' 后的 rawTags 赋值给 tags
    std::string tags = rawTags.substr(3);

    // 创建对话框
    // 创建 newTags 字符串并初始化为0，用于存储用户输入的文本，并将它作为对话框函数的参数
    WCHAR wNewTags[MAX_PATH] = { 0 };
    HWND hwndDlg = CreateDialogParamW(hInstance, MAKEINTRESOURCE(IDD_INPUTDIALOG), NULL, TagEditDialog, (LPARAM)wNewTags);
    SetDlgItemTextA(hwndDlg, IDC_INPUTBOX, tags.c_str());

    // 设置对话框的标题
    SetWindowTextW(hwndDlg, L"编辑标签");

    // 设置对话框的样式，使得不显示关闭按键
    SetWindowLong(hwndDlg, GWL_STYLE, GetWindowLong(hwndDlg, GWL_STYLE) & ~WS_SYSMENU);

    // 计算对话框的位置，使其出现在屏幕中央
    RECT rect;
    GetWindowRect(hwndDlg, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    MoveWindow(hwndDlg, x, y, width, height, TRUE);

    // Focus 到输入框并选中当前内容
    HWND hwndInputBox = GetDlgItem(hwndDlg, IDC_INPUTBOX);
    SetFocus(hwndInputBox);
    SendMessage(hwndInputBox, EM_SETSEL, 0, -1);

    // 显示对话框
    ShowWindow(hwndDlg, SW_SHOW);

    // 进入消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hwndDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    std::wstring wRawNewTags(wNewTags);
    if (wRawNewTags.empty()) {
        if(ini.Delete("{F29F85E0-4FF9-1068-AB91-08002B27B3D9}", "Prop5", true) < 0) { return -1; }
    }
    else {
        wRawNewTags = L"31," + wRawNewTags;
        //将rawNewTags 转换为 utf-8 编码
        std::string rawNewTags = convert_wchar_to_utf8(wRawNewTags.c_str());

        if (ini.SetValue("{F29F85E0-4FF9-1068-AB91-08002B27B3D9}", "Prop5", rawNewTags.c_str()) < 0) { return -1; }
    }

    // 将修改后的 desktop.ini 写入临时文件夹以备 vbs 脚本使用，因为 vbs 的 folder.CopyHere 能有效地刷新文件夹信息
    WCHAR tmpRoot[MAX_PATH];
    GetTempPathW(MAX_PATH, tmpRoot);
    WCHAR tmpFolder[MAX_PATH];
    GetTempFileNameW(tmpRoot, L"tag", 0, tmpFolder);
    DeleteFileW(tmpFolder);
    CreateDirectoryW(tmpFolder, NULL);
    WCHAR tmpINIPath[MAX_PATH];
    PathCombineW(tmpINIPath, tmpFolder, L"desktop.ini");
    if (ini.SaveFile(tmpINIPath) < 0) { return -1; }

    // 创建调用 folder.MoveHere 的 vbs 脚本并运行
#pragma warning( push )
#pragma warning( disable : 6387 )
    WCHAR tmpVBScriptPath[MAX_PATH];
    PathCombineW(tmpVBScriptPath, tmpFolder, L"copy.vbs");
    FILE* vbsFile;
    if (_wfopen_s(&vbsFile, tmpVBScriptPath, L"w+,ccs=UTF-8") == 0) {
        fwprintf(vbsFile, L"%s", L"set shell = CreateObject(\"Shell.Application\")\n");
        fwprintf(vbsFile, L"%s", L"set folder = shell.NameSpace(\"");
        fwprintf(vbsFile, L"%s", directoryName);
        fwprintf(vbsFile, L"%s", L"\")\n");
        fwprintf(vbsFile, L"%s", L"folder.MoveHere \"");
        fwprintf(vbsFile, L"%s", tmpINIPath);
        fwprintf(vbsFile, L"%s", L"\", 4+16+1024");
        fclose(vbsFile);
    }
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = L"open";
    ShExecInfo.lpFile = tmpVBScriptPath;
    ShExecInfo.nShow = SW_HIDE;
    ShellExecuteExW(&ShExecInfo);
    HANDLE hProcess = ShExecInfo.hProcess;
    WaitForSingleObject(hProcess, INFINITE);
    CloseHandle(hProcess);
#pragma warning( pop ) 

    // 为 desktop.ini 添加 hidden 和 system 属性
    DWORD desktopINIAttributes = GetFileAttributesW(desktopINIPath);
    SetFileAttributesW(desktopINIPath, desktopINIAttributes | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    
    // 删除临时文件夹
    DeleteFileW(tmpVBScriptPath);
    RemoveDirectoryW(tmpFolder);

	return 0;
}